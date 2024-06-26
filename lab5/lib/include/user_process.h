#ifndef __THREAD_H
#define __THREAD_H

#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE				4096

#define MAX_NUM_PROCESS				64 

#define FIRST_TASK task[0]
#define LAST_TASK task[TASK_MAX_NUM-1]

#define TASK_RUNNING				0

// struct callee_saved_reg {
//     unsigned long x19;
// 	unsigned long x20;
// 	unsigned long x21;
// 	unsigned long x22;
// 	unsigned long x23;
// 	unsigned long x24;
// 	unsigned long x25;
// 	unsigned long x26;
// 	unsigned long x27;
// 	unsigned long x28;
// };

// struct el0_context {
//     struct callee_saved_reg reg;
// 	unsigned long fp;   // x29
// 	unsigned long lr;   // x30, link register for function call
// 	unsigned long sp;

//     unsigned long user_stack;
// 	unsigned long kernel_stack;
    
//     unsigned int id;
//     unsigned int is_dead;
//     struct el0_context* next;
// };

struct pt_regs { // trap frame
    unsigned long regs[31];     //register
    // unsigned long sp;           //stack pointer
    // unsigned long pc;           //progeam counter; kernel_exit will copy pc to the elr_el1 register, thus making sure that we will return to the pc address after performing exception return.
    // unsigned long pstate;       //process state; This field will be copied to spsr_el1 by the kernel_exit and becomes the processor state after exception return is completed.
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;
};

void fork_test();

#endif
#endif