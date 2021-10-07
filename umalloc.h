#include <stdlib.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bits 1-3 are unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    //header size is now 24!**
    size_t block_size_alloc;

    struct memory_block_struct *prev;
    //free: *next points to next free node
    //allocated: *next points to allocation value that determines whether it is allocated or not
    struct memory_block_struct *next;

} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c
bool is_allocated(memory_block_t *block);
void allocate(memory_block_t *block);
void deallocate(memory_block_t *block);
size_t get_size(memory_block_t *block);
memory_block_t *get_next(memory_block_t *block);
void put_block(memory_block_t *block, size_t size, bool alloc);
void *get_payload(memory_block_t *block);
memory_block_t *get_block(void *payload);

//changed return types and parameters
memory_block_t *find(size_t size);
void extend();
memory_block_t *split(memory_block_t *block, size_t size);
void coalesce(memory_block_t *block);


// Portion that may not be edited
int uinit();
void *umalloc(size_t size);
void ufree(void *ptr);