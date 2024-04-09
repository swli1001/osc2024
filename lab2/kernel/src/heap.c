#include "heap.h"
#define NULL ((void*)0)      // Define NULL pointer
extern char _heap_top; // Start of heap, provided by the linker script
extern char _heap_end; // End of heap, provided by the linker script
#define HEAP_SIZE (1024 * 1024) // Define heap size explicitly if needed

typedef struct block_header {
    struct block_header* next;
    unsigned int size;
    char free;
} block_header_t;

#define ALIGN 8
#define BLOCK_HEADER_SIZE ((sizeof(block_header_t) + (ALIGN-1)) & ~(ALIGN-1))
#define ALIGN_SIZE(size) (((size) + (ALIGN-1)) & ~(ALIGN-1))

static block_header_t* head = NULL;
static block_header_t* tail = NULL;

void* malloc(unsigned int size) {
    size = ALIGN_SIZE(size) + BLOCK_HEADER_SIZE;

    if (head == NULL) { // First call to malloc
        head = (block_header_t*)&_heap_top;
        if ((char*)head + size > &_heap_end) return NULL; // Out of heap space

        head->size = size - BLOCK_HEADER_SIZE;
        head->free = 0;
        head->next = NULL;
        tail = head;
        return (char*)head + BLOCK_HEADER_SIZE;
    }

    block_header_t* current = head;
    while (current) {
        if (current->free && current->size >= size - BLOCK_HEADER_SIZE) {
            current->free = 0;
            return (char*)current + BLOCK_HEADER_SIZE;
        }
        current = current->next;
    }

    // No suitable block found, expand heap
    if ((char*)tail + tail->size + size > &_heap_end) return NULL; // Out of memory

    current = (block_header_t*)((char*)tail + tail->size);
    current->size = size - BLOCK_HEADER_SIZE;
    current->free = 0;
    current->next = NULL;
    tail->next = current;
    tail = current;

    return (char*)current + BLOCK_HEADER_SIZE;
}

void free(void* ptr) {
    if (!ptr) return;

    // Simple approach: Mark the block as free.
    block_header_t* block = (block_header_t*)((char*)ptr - BLOCK_HEADER_SIZE);
    block->free = 1;
    // Note: Coalescing adjacent free blocks is not implemented here.
}

