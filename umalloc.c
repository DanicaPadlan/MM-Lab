#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Danica Padlan - dmp3357" ANSI_RESET;

/*  ~Overall Implementation Information~
 *  The free blocks are in a double-linked list with pointers to the prev and next block.
 *  The free list is sorted in ascending memory address order.
 *
 *  When allocating, delinks the allocated block and adjusts its prev and next block to point to each other
 *  Allocater finds and returns first fitting block for requested size.
 *
 *  When freeing block, inserts block in free list in accordance to memory address and
 *  checks for neighboring blocks to coalesce with
 */

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

//A sample pointer to the start of the free list.
memory_block_t *free_head; 

//Keeps track of last free block
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
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * and prev field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL; 
    block->prev = NULL;
}

/* 
 * get_payload - gets the payload of the block. (Revised to support 32 byte header)
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);

    //returns payload address (address of where to start putting storage)
    return (void*)(block + 1);
}

/* 
 * get_block - given a payload, returns the block. (Revised to support 32 byter header)
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);

    //returns address of header (holds block_size_alloc, prev, and next information)
    return ((memory_block_t *)payload) - 1;
}

/* 
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

/* 
 * insert - finds spot to insert block in ascending order in accordance to memory address
 */
void insert(memory_block_t* curBlock){
    //pre-condition: curBlock cannot be NULL
    assert(curBlock != NULL);

    //special case: free list is empty
    if(free_head == NULL){

        //updates curBlock to free_head and last_free
        free_head = curBlock;
        last_free = free_head;

        //nulls out prev and next pointers since curBlock is the only block in the list
        free_head->prev = NULL;
        free_head->next = NULL;
        return;
    } 

    //special case: curBlock address is less than free_head
    if(curBlock < free_head){

        //set old freehead's prev to curblock
        free_head->prev = curBlock;

        //set curBlock's next to free head and prev to NULL 
        curBlock->next = free_head;
        curBlock->prev = NULL;

        //set new free_head to put curBlock as first block in list
        free_head = curBlock;
        return;
    }

    //special case: address greater than last_free
    if(curBlock > last_free){

        //set old last_free's next to curBlock
        last_free->next = curBlock;

        //sets curBlock's prev to last_free and next to NULL
        curBlock->prev = last_free;
        curBlock->next = NULL;

        //sets new last_free to put curBlock last in list
        last_free = curBlock;
        return;
    }

    //general case: inserts in middle of list, means prev and next are non-NULL blocks
    //arithmetic determines which end of list to start in
    //if curBlock is closer to free_head (aka curBlock - free_head) < (last_free - curBlock) then start looking at free_head
    //if curBlock is closer to last_free (aka curBlock - free_head) > (last_free - curBlock) then start looking at last_free
    memory_block_t* curMemory = (curBlock - free_head) < (last_free - curBlock) ? free_head : last_free;

    //traverses blocks from the beginning of free list
    if(curMemory == free_head){
        while(curMemory && curMemory->next != NULL){

            //checking for addresses to be inserted between
            if(curMemory < curBlock && curBlock < curMemory->next){

                //insert in between the two blocks, changes curMemory's next prev 
                //and curBlock's next to each other
                curMemory->next->prev = curBlock;
                curBlock->next = curMemory->next;

                //changes curMemory's next and curBlock's prev to each other
                curMemory->next = curBlock;
                curBlock->prev = curMemory;
                return;
            }
            curMemory = curMemory->next;
        }
    } 

    //else traverses blocks from the end of free list
    while(curMemory && curMemory->prev != NULL){

            //checking for addresses to be inserted between
            if(curMemory->prev < curBlock && curBlock < curMemory){

                //insert in between the two blocks, changes curMemory's prev next 
                //and curBlock's prev to each other
                curMemory->prev->next = curBlock;
                curBlock->prev = curMemory->prev;

                //changes curMemory's prev and curBlock's next to each other
                curMemory->prev = curBlock;
                curBlock->next = curMemory;
                return;
            }
            curMemory = curMemory->prev;
        }
}

/* 
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    //get new heap pool for more memory storage
    memory_block_t* temp = csbrk(size + (PAGESIZE/2));

    //initializing header for new heap pool
    put_block(temp, size + (PAGESIZE/2), false);

    //we know its greater than usual so always add it to the end of the list
    last_free->next = temp;
    temp->prev = last_free;
    last_free = temp;

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

        //checks if block is greater or equal to size AND potential leftover block 
        //is big enough to store another header and payload addresses to avoid out-of-bounds SEGFAULTS
        //or if block fits the size requirement perfectly
        if((get_size(curMemory) > size && (get_size(curMemory) - size) > sizeof(memory_block_t)) || 
            get_size(curMemory) == size){
            return curMemory;
        }
        curMemory = curMemory->next;
    }

    //special case: no block can hold requested size, must call extend for new block
    return extend(size); 
}

/*  
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    //find size of leftover block after allocating part of the block
    size_t leftoverSize = get_size(block) - size;

    //allocating last splitted portion of the block, keeping first half in free list
    //calculates allocating block address
    memory_block_t* allocatedBlock = (memory_block_t*) ((char*) block + leftoverSize);
    
    //sets leftover block to new size
    block->block_size_alloc = leftoverSize;

    //sets allocated blocks size and allocated boolean
    put_block(allocatedBlock, size, true);
    return allocatedBlock;
}

/*
 * coalesce - coalesces a free memory block with neighbors
 */
void coalesce(memory_block_t *block) {
    //checking if previous node can merge with block
    if(block->prev != NULL){

        //pointer arithmetic to find previous neighboring address
        char* neighborBlock = (char*) block->prev;
        neighborBlock = neighborBlock + get_size(block->prev);

        //checks if result of pointer arithmetic equals block
        if(((memory_block_t*) neighborBlock) == block){

            //calculates new size of merged blocks
            size_t mergeSize = get_size(block->prev) + get_size(block);
            block->prev->next = block->next;

            //special case: block is last_free
            if(last_free == block){

                //sets prev block to new last_free
                last_free = block->prev;

            //else next is not null    
            } else if(block->next != NULL){

                //links next block to prev block
                block->next->prev = block->prev;
            }

            //updates block's new merged size
            block->prev->block_size_alloc = mergeSize;

            //dereferences block to prev block to update for next check
            block = block->prev;

        } 
    }

    //checking for next pointer
    if(block->next != NULL){

        //pointer arithmetic to find next neighboring address
        char* neighborBlock = (char*) block;
        neighborBlock = neighborBlock + get_size(block);

        //checks if next block is a neighbor
        if(((memory_block_t*) neighborBlock) == block->next){

            //calculates new size of merge block
            size_t mergeSize = get_size(block) + get_size(block->next);

            //special case: set the next block's prev and next pointers for last_free
            if(last_free == block->next){

                //sets prev block to last_free
                last_free = last_free->prev;

            //links next->next block to current block
            } else if(block->next->next != NULL){
                block->next->next->prev = block;
            }

            //updates block's next
            block->next = block->next->next;

            //updates block's new merged size
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
    free_head = csbrk(PAGESIZE);

    //updates last_free to memory in free_head since lone heap
    last_free = free_head;

    //initializing header
    put_block(free_head,PAGESIZE, false);

    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    //pre-condition where size must be greater than 0
    assert(size > 0);

    //request for desired size + header size (32 or size of memory_block_t)
    int appSize = ALIGN(size + sizeof(memory_block_t));

    //find returns the address with headers 
    memory_block_t* availBlock = find(appSize); 

    //check for split case
    if(get_size(availBlock) > appSize){

        //splits leftover block from allocating block
        return get_payload(split(availBlock, appSize));

    //if it is not split case   
    } 

    //sets up block as allocated
    allocate(availBlock);

    //special case if allocating free_head
    if(free_head == availBlock){

        //updates free_head to next block to delink availBlock
        free_head = free_head->next;
        free_head->prev = NULL;
    } 

    //special case if allocating last_free
    if(last_free == availBlock){

        //updates last_free to prev block to delink availBlock
        last_free = availBlock->prev;
        last_free->next = NULL;
        availBlock->prev = last_free;
    }

    //sets block's prev and next to point to each other if not NULL
    //to delink availBlock from free list
    if(availBlock->prev != NULL){
        availBlock->prev->next = availBlock->next;
    }
    if(availBlock->next != NULL){
        availBlock->next->prev = availBlock->prev;
    }

    //dereferences availBlock's next and prev
    availBlock->next = NULL;
    availBlock->prev = NULL;

    //returns payload address to user
    return get_payload(availBlock);  
}

/*
 * ufree -  frees the memory payload space pointed to by ptr, which must have been called
 * by a previous call to malloc. 
 */
void ufree(void *ptr) {
    //pre-condition: ptr cannot be NULL
    assert(ptr != NULL);

    //get block address first
    memory_block_t* curHeader = get_block(ptr);

    //turns allocated block to deallocated 
    deallocate(curHeader);

    //inserts the block in free list in accordance to memory address
    insert(curHeader);

    //checks if neighbors can be merged
    coalesce(curHeader);
    return;
}