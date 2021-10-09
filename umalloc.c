#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Danica Padlan - dmp3357" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

// A sample pointer to the start of the free list.
//Only holds free nodes, user is in charge of keeping allocated pointers
memory_block_t *free_head; 
memory_block_t* last_free;

//have a variable for max header address that gets updated a lot and checked to make sure a space is not out of range


/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL; //segfault happening
    block->prev = NULL;
}

//adjust payload address
/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    //returns address payload address (address of where to start putting storage)
    char* p = (char*) block;
    p = p + ALIGNMENT;
    return (void*) p;
    //return (void*)(block + 1);
}

//adjusted method
/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    //returns address of header (holds size and next information)
    char* p = (char*) payload;
    p = p - ALIGNMENT;
    return (memory_block_t*) p;
    //return ((memory_block_t *)payload) - 1;
}

//MY CODING AREA
/*
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

//debugging print
/*
void print_free(){
    memory_block_t* cur = free_head;
    printf("Printing free list\n");
    while(cur){
        printf("Starting Memory Address: %p, Ending Memory Address is: %p ~ Prev: %p ~ Next: %p   \n", cur, (void*) ((char*)cur + get_size(cur)), cur->prev, cur->next );
        cur = cur->next;
    }
    return;
}
*/

/*
 * insert - finds spot to insert block in accordance to memory address
 */
void insert(memory_block_t* curBlock){

    if(free_head == NULL){
        free_head = curBlock;
        free_head->prev = NULL;
        free_head->next = NULL;
        return;
    }

    //special case/less than free head
    if(curBlock < free_head){

        //set old freehead's prev to curblock
        free_head->prev = curBlock;
        //set curblock's next to free head
        curBlock->next = free_head;
        curBlock->prev = NULL;
        //set new freehead to curblock
        free_head = curBlock;

        return;
    }

    //special cases for last //added this*** not sure if this solves the problem////
    if(curBlock > last_free){
        last_free->next = curBlock;
        curBlock->prev = last_free;
        curBlock->next = NULL;

        last_free = curBlock;
        return;
    }

    //should be for middle inserts only
    memory_block_t* curMemory = free_head;
    while(curMemory && curMemory->next != NULL){
        //all other mid cases by checking cur memory and its next
        if(curBlock > curMemory && curBlock < curMemory->next){

            //insert in between the two blocks, changes curMemory's next prev and curBlock's next
            curMemory->next->prev = curBlock;
            curBlock->next = curMemory->next;

            //changes curMemory's next and curBlock's prev
            curMemory->next = curBlock;
            curBlock->prev = curMemory;
            return;

        }
        curMemory = curMemory->next;

    }
    return;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend() {
    //call csbrk like in uinit
    memory_block_t* temp = csbrk(16 * PAGESIZE);
    //initializing header
    put_block(temp, 16 * PAGESIZE, false);

    //insert memory addressin free list
    insert(temp);

    //returns new heap pool 
    return temp;
}


//first fit algorithm
/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {

    //starts searching in beginning of memory header list
    memory_block_t* curMemory = free_head;

    //run loop while curMemory is not null
    while(curMemory){

        //checking size if size is exact
        if(get_size(curMemory) >= size){
            return curMemory;
        }
        curMemory = curMemory->next;
    }

    //past this point, no blocks can fit desired size, must call extend
    return extend();
}

//segfault??
//recheck, middle manipulation should be 
//do after to consider special case**
/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    //gets new size for new split block, will be used to fill information in the new splitted header
    size_t newSize = get_size(block) - size;

    //gets header of the block
    char* p = (char*) block;
    //header address + size needed for allocating block (already includes header val)
    p = p + size;

    //gives in header address and new size to intialize new block 
    put_block((memory_block_t*) p, newSize, false); //segfault

    //update cur block's size
    block->block_size_alloc = size;

    //setting up new block
    //sets up block as allocated in its space address memory
    allocate(block);

    if(free_head == block){
        //printf("changing free head\n");
        free_head = (void*) p;
        free_head->next = block->next;
        free_head->prev = NULL;
        //never know if free head is connected dont assume
        if(block->next != NULL){
            block->next->prev = free_head;
        }
        
        //special case if free_head is also the last_free where prev is null then,,
        if(last_free == block){
            last_free = free_head;
        }

    //if free_head is not the last header
    } else if(last_free == block){
        //printf("changing last free\n");
        last_free = (void*) p;
        last_free->prev = block->prev;
        last_free->next = NULL;
        if(block->prev != NULL){
            block->prev->next = last_free;
        }

    //middle list cases update next and prev blocks
    } else{
        ((memory_block_t*) p)->prev = block->prev;
        block->prev->next = (void*) p;
        ((memory_block_t*) p)->next = block->next;
        block->next->prev = (void*) p;
    }

    //returns current block
    return block;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
void coalesce(memory_block_t *block) {
    //pre-condition, block cannot be null
    if(block == NULL){
        //printf("Null block\n");
    }

    //checking if previous node can merge with cur node
    if(block->prev != NULL){
        //pointer arithmetic attempt to reach cur block
        char* p = (char*) block->prev;
        p = p + get_size(block->prev);

        //checks if result of pointer arithmetic equals cur block
        if(((memory_block_t*) p) == block){
            //calculates new size of merged blocks
            size_t mergeSize = get_size(block->prev) + get_size(block);
            block->prev->next = block->next;
            //means next block is null
            if(last_free == block){
                last_free = block->prev;
            //else next is not null    
            } else if(block->next != NULL){
                block->next->prev = block->prev;
            }
            //sets new size of block-> prev 
            block->prev->block_size_alloc = mergeSize;
            //dereferences block to previous block
            block = block->prev;

        }
    }

    //checking for next pointer
    if(block->next != NULL){
        //pointer arithmetic attempt to reach next block
        char* p = (char*) block;
        p = p + get_size(block);

        //checks if next block is a neighbor
        if(((memory_block_t*) p) == block->next){
            //calculates new size of merge block
            size_t mergeSize = get_size(block) + get_size(block->next);

            //special checks to set the next block's prev and next pointers
            if(last_free == block->next){
                last_free = last_free->prev;
            } else if(block->next != NULL){
                block->next->prev = block;
            }

            //updates block's next
            block->next = block->next->next;
            block->block_size_alloc = mergeSize;
        }
        
        
    }
    return;
}


//deals with memory
/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    //call csbrk to initialize heap 
    //sets memory address to free_head
    free_head = csbrk( 16 * PAGESIZE);

    //initializing header
    put_block(free_head, 16 * PAGESIZE, false);

    //updates last_free to memory in free_head since lone heap
    last_free = free_head;

    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    //pre-conditiong where size must be greater than 0
    if(size <= 0){
        //return error for now null
        return NULL;
    }

    //request for desired size, must add in header size (32 or ALIGNMENT)
    int appSize = ALIGN(size + ALIGNMENT);


    //find returns the address with headers 
    memory_block_t* availBlock = find(appSize); 

    //check if availBlock is not null
    if(availBlock != NULL){

        //check for split case
        if(get_size(availBlock) > appSize){
            //call for split case, will return allocate memory here
            split(availBlock, appSize);

        //if it is not split    
        } else{
            //sets up block as allocated
            allocate(availBlock);

            //special case if allocating free_head
            if(free_head == availBlock){
                //just update block
                //changes free to free->next
                free_head = free_head->next;
                free_head->prev = NULL;
            } 

            //special case if allocating last_free
            if(last_free == availBlock){
                last_free = availBlock->prev;
                last_free->next = NULL;
                availBlock->prev = last_free;
            }

            //sets its prev and next to point to each other
            if(availBlock->prev != NULL){
                availBlock->prev->next = availBlock->next;
            }
            if(availBlock->next != NULL){
                availBlock->next->prev = availBlock->prev;
            }
            
        }
        //unlinks avilBlock's next and prev, moved outside non-split special case
        availBlock->next = NULL;
        availBlock->prev = NULL;

        //returns payload address to user
        return get_payload(availBlock);   
    } 
    return NULL;
}

/*
 * ufree -  frees the memory payload space pointed to by ptr, which must have been called
 * by a previous call to malloc. 
 */
void ufree(void *ptr) {
    if(ptr == NULL){
        //throw error, returning null for now 
        return;
    }
    //get block address first
    memory_block_t* curHeader = get_block(ptr);
    deallocate(curHeader);

    //after deallocating, find perfect spot to insert the block in accordance to memory address
    insert(curHeader);

    //after fit in spot, check if neighbors can be merged
    coalesce(curHeader);
    return;
}