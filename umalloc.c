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
int ALIGNMENT_PAD = ALIGNMENT * 2;


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
}

//adjust payload address
/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    //returns address payload address (address of where to start putting storage)
    char* p = (char*) block;
    p = p + ALIGNMENT_PAD;
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
    p = p - ALIGNMENT_PAD;
    return (memory_block_t*) p;
    //return ((memory_block_t *)payload) - 1;
}

//MY CODING AREA
/*
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

//debugging print
void print_free(){
    memory_block_t* cur = free_head;
    printf("Printing free list\n");
    while(cur){
        printf("Starting Memory Address: %p, Ending Memory Address is: %p ~ Prev: %p ~ Next: %p   \n", cur, (void*) ((char*)cur + get_size(cur)), cur->prev, cur->next );
        cur = cur->next;
    }
    return;
}

//*****recheck
//implement best fit algorithm
/*
 * insert - finds spot to insert block in accordance to memory address
 */
void insert(memory_block_t* curBlock){
    //loop through free list to find good spot
    //check by seeing if current memory and previous memory are less and greater than address
    //special empty list case
    //printf("performing insert, the current free block: %p\n", free_head);

    if(free_head == NULL){
        //printf("inserting %p in free_head when list is empty\n", curBlock);
        free_head = curBlock;
        free_head->prev = NULL;
        free_head->next = NULL;
        return;
    }

    //special case/less than free head
    if(curBlock < free_head){
        //printf("%p is less than free_head: %p\n", curBlock, free_head);

        //set old freehead's prev to curblock
        free_head->prev = curBlock;
        //set curblock's next to free head
        curBlock->next = free_head;
        curBlock->prev = NULL;
        //set new freehead to curblock
        free_head = curBlock;

        //printf("%p 's next memory address is %p\n", free_head, free_head->next);
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
    //problem is here***
    memory_block_t* curMemory = free_head;
    while(curMemory && curMemory->next != NULL){
        //printf("looping through free list cur block: %p\n", curMemory);
        //all other mid cases by checking cur memory and its next
        if(curBlock > curMemory && curBlock < curMemory->next){
            //printf("%p is greater than curMemory: %p and less than curNext: %p\n", curBlock, curMemory, curMemory->next);
            //insert in between the two blocks
            //changes curMemory's next prev and curBlock's next
            curMemory->next->prev = curBlock;
            curBlock->next = curMemory->next;

            //changes curMemory's next and curBlock's prev
            curMemory->next = curBlock;
            curBlock->prev = curMemory;
            return;

        }
        curMemory = curMemory->next;

    }
    //printf("%p is the last block, cannot find a perfect place to insert, Will add to the end\n", curBlock);
    //printf("the old last free is %p\n", last_free);

    //if it reaches this point, set to last header
    //pass this point, at last free block and no next 
    //potential problem: assume the block is bigger than curMemory
    /*
    curMemory->next = curBlock;
    curBlock->prev = curMemory;
    curBlock->next = NULL;

    //sets curBlock to last block
    last_free = curMemory;
    */

    //printf("the new last free is %p\n", last_free);
    return;
}

//FOCUS
//have not called it yet
//dont want to return anything atm 
/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend() {
    //call csbrk like in uinit
    memory_block_t* temp = csbrk(16 * PAGESIZE);
    //initializing header
    put_block(temp, 16 * PAGESIZE, false);
    //printf("new heap pool extension: %p\n", temp);
    //printf("before insert *first header is %p and last free is %p\n", free_head, last_free);
    //insert memory in right spot
    insert(temp);
    //printf("after extend's call on insert\n");
    //print_free();
    //printf("after insert *first header is %p and last free is %p\n\n", free_head, last_free);

    return temp;
}

//implement best fit algorithm
/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {

    //debugging to see free list
    //printf("In find(), here's the list\n");
    //print_free();

    //starts searching in beginning of memory header list
    memory_block_t* curMemory = free_head;
    //printf("looking for block to fit size: %li\n", size);
    //run loop while curMemory is not null, goes forward one way
    while(curMemory){
        //if appropriate total size (header + needed storage memory), return pointer
        //printf("checking current address at: %p\n", curMemory);

        //size includes header
        //printf("checking current size: %li\n", get_size(curMemory));

        //checking size if size is exact
        if(get_size(curMemory) >= size){
            //debugging
            if(!is_allocated(curMemory)){
            //debugging reasons
            //printf("found the perfect address! size is %li\n", get_size(curMemory));
            //printf("address is: %p \n", curMemory);
            //returns block/header pointer
            return curMemory;
            } else{
                //printf("memory: %p is allocated\n", curMemory);
            }


            
        }
        //printf("still looking\n");
        curMemory = curMemory->next;
    }

    //call extend in here?

    //printf("cant find must extend by calling extend\n");
    return extend();
}

//recheck, middle manipulation should be 
//do after to consider special case**
/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    //gets new size for new split block, will be used to fill information in the new splitted header
    size_t newSize = get_size(block) - size;
    //printf("before splitting\n");
    //print_free();


    //debugging
    /*
    printf("block size: %li\n", get_size(block));
    printf("leftover size: %li\n", newSize);
    */

    //gets header of the block
    char* p = (char*) block;
    //header address + size needed for allocating block (already includes header val)
    p = p + size;

    //gives in header address and new size to new block 
    put_block((void*) p, newSize, false); //segfault
    //debugging
    //printf("new split block at get_block: %p\n", (void*) p);
    //printf("size of split block is: %li\n", get_size( (void*) p));

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
        //print_free();

    //if free_head is not the last header
    } else if(last_free == block){
        //printf("changing last free\n");
        last_free = (void*) p;
        last_free->prev = block->prev;
        last_free->next = NULL;
        if(block->prev != NULL){
            block->prev->next = last_free;
        }
        //print_free();

    //problems occur here
    //middle list cases, !!must have prev and next!!    
    } else{
        //printf("checking middle\n");
        //printf("block's prev is %p and next is %p, ", block->prev, block->next);

        ((memory_block_t*) p)->prev = block->prev;
        block->prev->next = (void*) p;
        ((memory_block_t*) p)->next = block->next;
        //segfault! shoudln't be segfault if setting up the right titles
        block->next->prev = (void*) p;
    }
    //delink current allocated block
    block->next = NULL;
    block->prev = NULL;

    //debugging
    /*
    printf("address of free_head: %p\n", free_head);
    printf("address of splitted block: %p\n", (void*) p);

    //after changing the allocated block
    printf("after allocating og block, split block size is: %li\n", get_size( (void*) p));
    
    //printf("size of allocated bloc: %li", get_size(block) - ALIGNMENT_PAD);
    */
    return block;
}

//have not called coalesce yet
/*
 * coalesce - coalesces a free memory block with neighbors.
 */
void coalesce(memory_block_t *block) {
    //pre-condition, block cannot be null
    if(block == NULL){
        printf("Null block\n");
    }

    //checking if previous node can merge with cur node
    if(block->prev != NULL){
        //pointer arithmetic attempt to reach cur block
        char* p = (char*) block->prev;
        p = p + get_size(block->prev);

        //checks if result of pointer arithmetic equals cur block
        if(((memory_block_t*) p) == block){
            //printf("merging block->prev %p && block %p\n", block->prev, block );

            //printf("prev %p and current block %p are neighbors!\n", block->prev, block);
            size_t mergeSize = get_size(block->prev) + get_size(block);
            block->prev->next = block->next;
            //means next block is null
            if(last_free == block){
                last_free = block->prev;
            //else next is not null    
            } else if(block->next != NULL){
                block->next->prev = block->prev;
            }
            //printf("block prev: %p's next is %p. Merge size is: %li\n",block->prev, block->prev->next, mergeSize);
            //sets new size of block->prev Note: put_block causes infinite loops which is weird
            block->prev->block_size_alloc = mergeSize;
            //does not delink previous block, makes infinite loop
            //delink 
            block = block->prev;
            //printf("merged block %p prev %p & next %p\n", block, block->prev, block->next);

        }
    }

    //***
    //checking for next pointer
    if(block->next != NULL){
        //pointer arithmetic attempt to reach next block
        char* p = (char*) block;
        p = p + get_size(block);

        if(((memory_block_t*) p) == block->next){
            //printf("cur block %p and next block %p are neighbors!\n", block, block->next);
            
            //fixed the logic check order
            size_t mergeSize = get_size(block) + get_size(block->next);
            if(last_free == block->next){
                //printf("last free is %p, i will update to %p\n", last_free, last_free->prev);
                last_free = last_free->prev;
            } else if(block->next != NULL){
                block->next->prev = block;
            }
            block->next = block->next->next;
            block->block_size_alloc = mergeSize;
            //printf("after coalescing block : %p's prev is %p and next is %p. Merge size is: %li\n",block, block->prev, block->next, mergeSize);
        }
        
        
    }
    //printf("***After coalescing***\n");
    //print_free();

    //void return
    return;
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
    free_head->prev = NULL;

    //took out last_free header**
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
    //printf("asked to give %li size block\n", size);
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
    //must add in header size (16/ALIGNMENT*2)
    int appSize = ALIGN(size + ALIGNMENT_PAD);

    //debugging print statement, including header count
    //printf("byte space needed: %i\n", appSize);

    //problem can be in find
    //find returns the address with headers i think, not the payload?
    //first case, getting free memory block header
    memory_block_t* availBlock = find(appSize); //checking find something is failing to find an available block

    //check if availBlock is not null
    if(availBlock != NULL){
        //work more on extend and split case check orderings!

        //check for split case
        if(get_size(availBlock) > appSize){
            //call for split case, will return allocate memory here
            //printf("gonna split\n");
            //printf("app size: %i\n", appSize);
            split(availBlock, appSize);
        //if it is not split    
        } else{
            //printf("size of block is just right\n");
            //sets up block as allocated
            //changed allocate(availBlock) to allocate(get(pay_load(availBlock)))
            allocate(availBlock);

            //printf("!!Allocated Availblock address: %p, Ends at: %p\n", availBlock, (void*)((char*) availBlock + get_size(availBlock)));

            //rewrite
            //means its the first block
            if(free_head == availBlock){
                //just update block
                //changes free to free->next
                free_head = free_head->next;
                free_head->prev = NULL;

            } 

            if(last_free == availBlock){
                last_free = availBlock->prev;
                last_free->next = NULL;
                availBlock->prev = last_free;
                //printf("last free was updated from %p to %p\n", availBlock, availBlock->prev);
                //something is linking 
            }

            //printf("availblock -> prev is %p && availblock -> next is %p\n", availBlock->next, availBlock->prev);

            //***it's not delinking here hmmm
            //sets its prev and next to point to each other
            //assumptions are bad
            if(availBlock->prev != NULL){
                availBlock->prev->next = availBlock->next;
            }
            if(availBlock->next != NULL){
                availBlock->next->prev = availBlock->prev;
            }

            //unlinks avilBlock's next and prev
            availBlock->next = NULL;
            availBlock->prev = NULL;
        }

        //printf("i am sending back block to user from %p to %p\n\n", availBlock, (void*) ((char*) availBlock + get_size(availBlock)));

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
    //printf("adding back %p! HERE'S CURRENT LIST\n", get_block(ptr));
    //print_free();
    //printf("checking back free list before returning block\n");
    //print_free();
    //printf("returning block: %p\n", get_block(ptr));
    if(ptr == NULL){
        //throw error, returning null for now 
        return;
    }
    //get block address first
    memory_block_t* curHeader = get_block(ptr);
    deallocate(curHeader);
    //after deallocating, find perfect spot to insert the block in accordance to memory address
    insert(curHeader);
    //printf("~~ after inserting!\n");
    //print_free();
    //printf("~first header is %p and last free is %p\n\n", free_head, last_free);

    coalesce(curHeader);
    //printf("** after coalescing!\n");
    //print_free();
    //call coalesce after figuring it out
    //printf("%p's prev is %p and next is %p\n", curHeader, curHeader->prev, curHeader->next );
    //printf("*first header is %p and last free is %p\n\n", free_head, last_free);

    return;
}