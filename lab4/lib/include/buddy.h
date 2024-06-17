#ifndef __BUDDY_H
#define __BUDDY_H

/**
 * the whole usable memory region 0x00000000 - 0x3B400000
 * 
 * reserved memory
 *      1. Spin tables for multicore boot (0x0000 - 0x1000)
 *      2. Kernel image in the physical memory
 *      3. Initramfs
 *      4. Devicetree
 *      5. Your simple allocator (startup allocator)
 */
#define MEM_START           (0x00000000)
#define MEM_END             (0x3B400000)
#define MEM_SIZE            (MEM_END - MEM_START)
#define FRAME_SIZE          (4096) // 4KB, 0x1000
#define MAX_ORDER           (6) // order = 0 ~ 6
#define FRAME_NUM           (MEM_SIZE / FRAME_SIZE) // 242688 = 64 * 3792
#define BUDDY_ARR_LEN       (1 << MAX_ORDER)

/** 
 * page status
 * >= 0 -> allocable, contiguous memory.  size = (2^val) * frame_size
 * -1   -> free, but it belongs to a larger contiguous memory block
 * -2   -> allocated
 * -3   -> reserved // TODO
*/
#define FRAME_ISBUDDY   -1
#define FRAME_ALLOCATED -2
#define FRAME_RESERVED  -3

typedef struct frame_node {
    struct frame_node *prev;
    struct frame_node *next;
} frame_node;

void* ini_malloc(unsigned int msize);
// void* get_physical_addr(void *addr);
// void* get_logical_addr(void *addr);

void buddy_init();
void* alloc_frame(unsigned int frame_num);
void free_frame(void* addr);
// void memory_reserve(void* start, void* end);
void frame_status_dump();

#endif