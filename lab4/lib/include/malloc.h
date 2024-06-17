#ifndef _MALLOC_H
#define _MALLOC_H

#define MAX_CHUNK_SIZE 1024
#define MAX_RESERVE_NUM 8 

void* my_malloc(int msize);
void memory_init();
void* malloc(unsigned int msize);

#endif