#ifndef __THREAD_H
#define __THREAD_H

#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE				4096

#define MAX_NUM_PROCESS				64 

#define FIRST_TASK task[0]
#define LAST_TASK task[TASK_MAX_NUM-1]

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

struct el0_context {
    struct callee_saved_reg reg;
	unsigned long fp;   // x29
	unsigned long lr;   // x30, link register for function call
	unsigned long sp;

    unsigned long elr_el1; // self add
    unsigned long sp_el0; // self add
    
    unsigned int id; // self add
    unsigned int is_dead; // self add
    struct cpu_context* next;
};


void fork_test();

#endif
#endif