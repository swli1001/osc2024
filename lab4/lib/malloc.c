#include "malloc.h"
#include "mini_uart.h"
#include "buddy.h"
#include "dyn_alloc.h"

extern char __end__, __HeapLimit;
static char *heapStartAddr = &__end__;
static char *heapEndAddr = &__HeapLimit;

static unsigned int reserve_fidx_list[MAX_RESERVE_NUM][2] = {0};
static unsigned int reserved_cnt = 0;

/**
 * lab 2
 */
void* my_malloc(int msize) {
    int avail_size = heapEndAddr - heapStartAddr;
    if(msize > avail_size) {
        uart_send_string("heap space running out\r\n");
        return (void*) 0;
    }
    char *ptr = heapStartAddr;
    heapStartAddr += msize;
    return ptr;
}

/**
 * lab 4 
 * 
 */

void memory_init() {
    buddy_init();
    dyn_alloc_init();
}

void* malloc(unsigned int msize) {
    unsigned int frame_num;
    if(msize <= MAX_CHUNK_SIZE) { return alloc_chunk(msize); }
    else {
        // pad msize to 4096
        for(frame_num = 0; frame_num < (1<<MAX_ORDER); frame_num++) {
            if(frame_num * FRAME_SIZE >= msize) { return alloc_frame(frame_num); }
        }
    }
    uart_send_string("[ERROR] malloc exceed max size\r\n");
    return (void*)0;
}

// void memory_reserve(void *start_addr, void *end_addr) {
//     if(reserved_cnt >= MAX_RESERVE_NUM) {
//         uart_send_string("[ERROR] memory reserve exceed max number\r\n");
//         return;
//     }

//     reserved_cnt++;
// }