#ifndef __THREAD_H
#define __THREAD_H

#define THREAD_CPU_CONTEXT			0 		// offset of thread_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE			4096
#define TASK_MAX_NUM		64 
#define TASK_RUNNING		0
#define TASK_DEAD			1
#define TASK_WAITING		2
#define TASK_MAX_PRIOROTY	999

struct callee_saved_reg {
    unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
};

struct thread_context {
    struct callee_saved_reg reg;
	unsigned long fp;   // x29
	unsigned long lr;   // x30, link register for function call
	unsigned long sp;
};

struct thread {
	struct thread_context context;
	char* user_stack;
	char* kernel_stack;	
	unsigned int priority;
    unsigned int id;
    unsigned int state;
	struct thread* prev;
    struct thread* next;
};

extern void thread_context_switch(void* prev, void* next);
extern struct thread* get_current_thread(void);

void thread_init();
int thread_create(void (*thread_func)(void));
void thread_schedule();
void thread_test();

#endif
#endif