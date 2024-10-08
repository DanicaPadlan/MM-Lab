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
    //header size is 32 bytes
    size_t block_size_alloc;

    //extra field is used for padding to make 
    //struct size 16 byte aligned
    size_t padding;

    //pointers for prev and next blocks
    struct memory_block_struct *prev;
    struct memory_block_struct *next;
    

} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c
bool is_allocated(memory_block_t *block);
void allocate(memory_block_t *block);
void deallocate(memory_block_t *block);
size_t get_size(memory_block_t *block);
void put_block(memory_block_t *block, size_t size, bool alloc);
void *get_payload(memory_block_t *block);
memory_block_t *get_block(void *payload);

memory_block_t *find(size_t size);
memory_block_t *extend(size_t size);
memory_block_t *split(memory_block_t *block, size_t size);
void coalesce(memory_block_t *block);


// Portion that may not be edited
int uinit();
void *umalloc(size_t size);
void ufree(void *ptr);