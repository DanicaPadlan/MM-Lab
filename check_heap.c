
#include "umalloc.h"
#include "stdio.h" 

//Place any variables needed here from umalloc.c as an extern.
extern memory_block_t *free_head;

// Check that all blocks in the free list are marked free.
int check_free(){
    memory_block_t *cur = free_head;

    //loops through the free list
    while (cur) {

        //If a block is marked allocated in the free list
        if (is_allocated(cur)) {

            //return -1, it is not a valid list
            printf("not all blocks are free in the list\n");
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}

//Checks if all blocks are multiples of 16
int check_mult(){
    memory_block_t *cur = free_head;

    //loops through the free list
    while(cur) {

        //if size is not aligned to 16
        if (get_size(cur) % ALIGNMENT != 0) {

            //return -1, it is not a valid list
            printf("not all block sizes are multiples of 16\n");
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}

//checks if free list is in memory addresses' ascending order
int check_ascending(){
    memory_block_t *cur = free_head;

    //loops through the free list
    while(cur){

        //checks based on previous node, checking free_head could be an invalid check
        if(cur != free_head){

            //checks if previous block's memories are greater than the current block
            if(cur->prev > cur){

                //if a block is not in the right order, return -1
                printf("not all blocks are in ascending order\n");
                printMemory();
                return -1;
            }
        }
        cur = cur->next;
    }
    return 0;
}

//checks for neighbors that could have been coalesced
int check_neighbors(){
    memory_block_t *cur = free_head;

    //loops through the free list
    while(cur && cur->next != NULL){

        //calculates the neighboring address through pointer arithmetic
        memory_block_t* addressCheck = (memory_block_t*) ((char*) cur + get_size(cur));

        //checks if the next block in free list is it's neighbor 
        if(addressCheck == cur->next){

            //if it is then not all blocks have been coalesced properly, return -1
            printf("not all neighbors are combined\n");
            printf("Cur Address: %p, Neighbor Address: %p\n", cur, cur->next);
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 * Required to be completed for checkpoint 1.
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    //if any of these tests do not return zero, it will return -1
    if(check_free() != 0 || check_mult() != 0 || check_ascending() != 0 || check_neighbors() != 0){
        printf("Failed tests\n");
        return -1;
    }
    //else it passes
    printf("No problems!\n");
    return 0;
}

