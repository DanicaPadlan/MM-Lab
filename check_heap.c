
#include "umalloc.h"
//take out later after checking!!!
#include "stdio.h"

//Place any variables needed here from umalloc.c as an extern.
extern memory_block_t *free_head;


//printf debugging
void printMemory(){
    memory_block_t* cur = free_head;
    while(cur){
        printf("Memory Address: %p, Block Size: %li\n", cur, get_size(cur));
        cur = cur->next;
    }
    return;

}

// Check that all blocks in the free list are marked free.
// If a block is marked allocated, return -1.
int check_free(){
    memory_block_t *cur = free_head;
    while (cur) {
        if (is_allocated(cur)) {
            printf("not all blocks are free in the list\n");
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}

//Checks if all blocks are multiples of 16
//if not, return -1
int check_mult(){
    memory_block_t *cur = free_head;
    while(cur) {
        if (get_size(cur) % ALIGNMENT != 0) {
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
    while(cur){
        if(cur != free_head){
            if(cur->prev > cur){
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
    while(cur && cur->next != NULL){
        memory_block_t* addressCheck = (memory_block_t*) ((char*) cur + get_size(cur));
        if(addressCheck == cur->next){
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
    //coalesce is a problem!!

    if(check_free() != 0 || check_mult() != 0 || check_ascending() != 0 || check_neighbors() != 0){
        printf("failed\n");
        return -1;
    }

    printf("No problems!\n");
    return 0;
}

