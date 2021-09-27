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

//check to make sure if okay**
memory_block_t* last_free;


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
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    //returns address payload address (address of where to start putting storage)
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    //returns address of header (holds size and next information)
    return ((memory_block_t *)payload) - 1;
}

/*
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

//find_prev: helps find previous node of current node 
//O(N) time
memory_block_t *find_prev(memory_block_t* goal){
    if(goal == NULL){
        //throw error instead but return null for mean time to prevent
        return NULL;
    }
    memory_block_t* curMemory = free_head;
    while(curMemory != NULL && curMemory->next != NULL){
        //check if pointer is same as goal 
        if(curMemory->next == goal){
            return curMemory; 
        }
        curMemory = curMemory->next;
    }
    return NULL;
}


/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    //starts searching in beginning of memory header list
    memory_block_t* curMemory = free_head;

    //run loop while curMemory is not null, goes forward one way
    while(curMemory != NULL){
        //if appropriate total size (header + needed storage memory), return pointer
        //printf("checking current address at: %p\n", curMemory);

        //size includes header
        //printf("checking current size: %li\n", get_size(curMemory));

        if( (get_size(curMemory)) >= size && get_size(curMemory) != 0){
            //debugging reasons
            //printf("found the perfect address! size is %li\n", get_size(curMemory));
            //printf("address is: %p \n", curMemory);
            //returns block/header pointer
            return curMemory;
        }
        curMemory = curMemory->next;
    }
    printf("cant find\n");
    return NULL;
}

//dont want to return anything atm 
/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    //PLAN
    //call csbrk like in uinit
    //call put_block
    //set last_header to point to new heap pool
    //set last_header to 
    //change value to ask from csbrk?
    memory_block_t* temp = csbrk(32* PAGESIZE);
    //initializing header
    put_block(temp, PAGESIZE, false);
    
    //set last_free's next to new heap pool
    last_free->next = temp;

    //sets last_free block to new heap 
    last_free = temp;

    //error check
    if(last_free == NULL){
        //finds intializing error
        //throw error!
    }

    //leave as null since dont want to return anything
    return NULL;
}

//do after to consider special case**
/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    //PLAN
    //subtract parameter desired size given in parameter
    //"create" new memory block header, set up the leftover size to its size, set new memory block's next to current memory block's next
    //set cur memory block's next to new memory and new size to parameter size
    //allocate block in here and then return the address of payload
    
    //potential problem
    //gets new size for new split block, will be used to fill information in the new splitted header
    size_t newSize = get_size(block) - size;
    //debugging
    /*
    printf("block size: %li\n", get_size(block));
    printf("leftover size: %li\n", newSize);
    */

    //gets header of the block
    char* p = (char*) block;
    //header address + size needed for allocating block (already includes header val)
    p = p + size;

    //newSize does not stick, possibly because of pointer?
    //gives in header address and new size
    put_block((void*) p, newSize, false);
    //debugging
    /*
    printf("new split block at get_block: %p\n", (void*) p);
    printf("size of split block is: %li\n", get_size( (void*) p));
    */

    //Note: try block pointers


    //get the new address to end of free list
    //change last free header's next to new block
    last_free->next = (void*) p;
    //new last free is the new block
    last_free = last_free->next;

    //update block's size
    block->block_size_alloc = size;

    //setting up new block
    //sets up block as allocated in its space address*** potential bug
    //changed allocate(block) to allocate(get(pay_load(block)))
    allocate(get_payload(block));

    //updating the free list

    //if block is the first one, 
    if(free_head == block){
        //move first_head pointer to next
        free_head = free_head->next;
    //else in the middle of the list    
    } else{
        //calling O(N) method to find previous node
        //skips and delinks block header
        find_prev(block)->next = block->next;

        //what to do with next? either null out or double check for allocation boolean
        //null out for now, dont think we need the double allocation check rn
        block->next = NULL;
    }

    //debugging
    /*
    printf("address of free_head: %p\n", free_head);
    printf("address of splitted block: %p\n", (void*) p);

    //after changing the allocated block
    printf("after allocating og block, split block size is: %li\n", get_size( (void*) p));
    */

    return block;
}

//figure out after split bc memory address
/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    return NULL;
}



/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    //PLAN
    //set free_header to null 
    //call csbrk to initialize heap 
    //sets memory address to free_head
    free_head = csbrk( 16 * PAGESIZE);
    //initializing header
    put_block(free_head, 16 * PAGESIZE, false);

    //keeps track of last_free header
    last_free = free_head;
    //error check
    if(free_head == NULL){
        //finds intializing error
        return -1;
    }
    
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    //debugging free_head
    /*
    printf("current free_head address: %p \n", free_head);
    printf("free head's size: %li\n", get_size(free_head));
    */


    if(size <= 0){
        //return error for now null
        return NULL;
    }

    //request for desired size, does not include header
    //must add in header size
    int appSize = ALIGN(size + ALIGNMENT);

    //debugging print statement, including header count
    //printf("byte space needed: %i\n", appSize);

    //problem can be in find
    //find returns the address with headers i think, not the payload?
    //first case, getting free memory block header
    memory_block_t* availBlock = find(appSize);

    //check if availBlock is not null
    if(availBlock != NULL){
        //work more on extend and split case check orderings!

        //check for split case
        if(get_size(availBlock) > appSize){
            //call for split case, will return allocate memory here
            split(availBlock, appSize);
        } else{
            //sets up block as allocated
            //changed allocate(availBlock) to allocate(get(pay_load(availBlock)))
            allocate(get_payload(availBlock));
            //removes block from free list, go through n times to find prev node
            //segfault bc might be first
            //means its the first block
            if(free_head == availBlock){
                //just update block
                free_head = availBlock->next;

            } else{
                //delinks block from list and
                find_prev(availBlock)->next = availBlock->next;
            }

            //unlinks avilBlock's next
            //what to do with next? either null out or double check for allocation boolean
            //null out for now, dont think we need the double allocation check rn
            availBlock->next = NULL;
        }
        
        //returns payload address to user
        return get_payload(availBlock);   
    } 
    //work on this last    
    //needs more memory! coalescing or extend case, check for else!
    /*
    else{
        printf("Need more memory!");
        //call coalesce to make sure we have room
        //coalesce(availBlock, appSize);
        //then call find
        //if find returns another null, call extend()
    }
    */
       
    return NULL;
}

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr) {
    //potential problem:
    //user returns memory to payload address***

    //PLAN 
    //deallocate it /"free"
    //add the header back into free
    //return;

    if(ptr == NULL){
        //throw error, returning null for now 
        return;
    }
    //get block address first
    memory_block_t* curHeader = get_block(ptr);

    //should clear block just incase of leftover values** how to do?

    //then deallocate it
    deallocate(curHeader);
    
    //take last_free and set it's next to this ptr
    last_free->next = curHeader;

    //sets last_free to newly freed block
    last_free = curHeader;

    return;
}