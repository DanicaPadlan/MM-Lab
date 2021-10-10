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

//A sample pointer to the start of the free list.
memory_block_t *free_head; 

//Keeps track of last free block
memory_block_t* last_free;

//constant alignment
int ALIGNMENT_PADDING = sizeof(memory_block_t);

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
    block->next = NULL; //segfault due to ALIGNMENT value
    block->prev = NULL;
}

//error with og !!!
/*
 * get_payload - gets the payload of the block. (Revised to support 32 byte header)
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    //returns address payload address (address of where to start putting storage)
    return (void*)(block + 1);

    /*
    assert(block != NULL);
    //returns address payload address (address of where to start putting storage)
    char* p = (char*) block;
    p = p + ALIGNMENT_PADDING;
    printf("block: %p and payload: %p\n", (void*) block, (void*)p); 
    return (void*) p;
    */
}

//**** arithmetic reference issue
/*
 * get_block - given a payload, returns the block. (Revised to support 32 byter header)
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    //returns address of header (holds size and next information)

    return ((memory_block_t *)payload) - 1;
    /*
    assert(payload != NULL);
    //returns address of header (holds size, prev, and next information)
    char* p = (char*) payload;
    p = p - ALIGNMENT_PADDING;
    return (memory_block_t*) p;
    */
}

/* 
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

/*
 * insert - finds spot to insert block in accordance to memory address
 */
void insert(memory_block_t* curBlock){
    //special case if free list is empty
    if(free_head == NULL){
        free_head = curBlock;
        free_head->prev = NULL;
        free_head->next = NULL;
        return;
    }

    //special case address less than free head
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

    //special cases address greater than last_free
    if(curBlock > last_free){
        last_free->next = curBlock;
        curBlock->prev = last_free;
        curBlock->next = NULL;

        last_free = curBlock;
        return;
    }

    //else performs middle of the list inserts
    memory_block_t* curMemory = free_head;
    while(curMemory && curMemory->next != NULL){

        //checking for addresses to be inserted between
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
memory_block_t *extend(size_t size) {
    //get new heap pool
    memory_block_t* temp = csbrk(size + PAGESIZE);

    //initializing header for new heap
    put_block(temp, size + PAGESIZE, false);

    //insert memory address in free list
    insert(temp);

    //returns new heap pool 
    return temp;
}

/*
 * find - finds a free block that can satisfy the umalloc request by using the first fit algorithm
 */
memory_block_t *find(size_t size) {
    //starts searching in beginning of memory header list
    memory_block_t* curMemory = free_head;

    //runs loop while curMemory is not null
    while(curMemory){

        //checking block has appropriate size //debugging is allocated
        if(get_size(curMemory) >= size){
            return curMemory;
        }
        curMemory = curMemory->next;
    }

    //past this point, no blocks can fit desired size, must call extend
    return extend(size);
}

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

    //error happens before here
    //gives in header address and new size to intialize new block 
    put_block((memory_block_t*) p, newSize, false); 

    //update cur block's size
    block->block_size_alloc = size;

    //sets up block as allocated in its space address memory
    allocate(block);

    //special case: allocated block is free_head 
    if(free_head == block){

        //updates free_head to newly split block
        free_head = (void*) p;
        free_head->next = block->next;
        free_head->prev = NULL;

        //checking for existence of next block
        if(block->next != NULL){
            block->next->prev = free_head;
        }
        
        //special check case: if free_head is also the last_free where prev is null then,,
        if(last_free == block){
            last_free = free_head;
        }

    //special case: last_free is the splitted block
    } else if(last_free == block){

        //updates last_free to newly split block
        last_free = (void*) p;
        last_free->prev = block->prev;
        last_free->next = NULL;

        //checks for existence of prev block
        if(block->prev != NULL){
            block->prev->next = last_free;
        }

    //general case: middle list cases update next and prev blocks
    } else{
        ((memory_block_t*) p)->prev = block->prev;
        block->prev->next = (void*) p;
        ((memory_block_t*) p)->next = block->next;
        block->next->prev = (void*) p; 
    }

    //nulls prev and next for allocated block
    block->next = NULL;
    block->prev = NULL;

    //returns allocated block
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
        //pointer arithmetic attempt to find neighboring address
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

        //pointer arithmetic to find neighboring address
        char* p = (char*) block;
        p = p + get_size(block);

        //checks if next block is a neighbor
        if(((memory_block_t*) p) == block->next){
            //printf("cur block and next merging %p and %p\n", block, block->next);

            //calculates new size of merge block
            size_t mergeSize = get_size(block) + get_size(block->next);

            //special checks to set the next block's prev and next pointers
            if(last_free == block->next){
                last_free = last_free->prev;

                //updated the logic from block->next to block->next->next
            } else if(block->next->next != NULL){
                block->next->next->prev = block;
            }

            //updates block's next
            block->next = block->next->next;
            block->block_size_alloc = mergeSize;
        } 
    }
    return;
}


/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    //call csbrk to initialize heap 
    //sets memory address to free_head
    free_head = csbrk( ALIGNMENT * PAGESIZE);

    //updates last_free to memory in free_head since lone heap
    last_free = free_head;

    //initializing header
    put_block(free_head, ALIGNMENT * PAGESIZE, false);

    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    //pre-condition where size must be greater than 0
    if(size <= 0){
        //return error for now null
        return NULL;
    }

    //request for desired size, must add in header size (32 or ALIGNMENT PAD)
    int appSize = ALIGN(size + ALIGNMENT_PADDING);

    //find returns the address with headers 
    memory_block_t* availBlock = find(appSize); 

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
    //unlinks availBlock's next and prev
    availBlock->next = NULL;
    availBlock->prev = NULL;

    //debugging comment
    //printf("returning %p to user\n", get_payload(availBlock));

    //returns payload address to user
    return get_payload(availBlock);  
}

/*
 * ufree -  frees the memory payload space pointed to by ptr, which must have been called
 * by a previous call to malloc. 
 */
void ufree(void *ptr) {
    //pre-condition, ptr cannot be null
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