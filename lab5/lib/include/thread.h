#ifndef __THREAD_H
#define __THREAD_H

#define THREAD_CPU_CONTEXT			0 		// offset of thread_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE				4096

#define TASK_MAX_NUM				64 

#define TASK_RUNNING				0

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
    // long state;	// the state of the currently running task, support TASK_RUNNING
	// long counter; // decreases by 1 each timer tick and when it reaches 0 another task is thread_scheduled
	// long priority; // When a new task is thread_scheduled its priority is copied to counter
	// long preempt_count; // if != 0 -> critical section, timer tick ignored, rethread_schedule not triggered
    unsigned int id; // self add
    unsigned int is_dead; // self add
    struct thread_context* next;
};

extern void thread_context_switch(void* prev, void* next);
extern struct thread_context* get_current_thread(void);

void thread_init();
void thread_schedule();
void thread_test();

#endif
#endif