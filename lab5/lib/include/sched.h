#ifndef __SCHED_H
#define __SCHED_H

#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE				4096

#define TASK_MAX_NUM				64 

#define FIRST_TASK task[0]
#define LAST_TASK task[TASK_MAX_NUM-1]

#define TASK_RUNNING				0

extern struct task_struct *current; // always points to the currently executing task
extern struct task_struct * task[TASK_MAX_NUM];
extern int nr_tasks; // contains the number of currently running tasks in the system

/**
 * in Linux, both thread and processes are just different types of tasks
 * 
 * cpu_context
 *      contains all registers that might be different between the switched tasks
 *      ARM calling convention: x0 - x18 can be overwritten by the called function
 * 
 * task_struct
 *      a struct that describes a process
 */

struct cpu_context {
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
	unsigned long fp;   // x29
	unsigned long sp;
	unsigned long pc;   // x30
};

struct task_struct {
	struct cpu_context cpu_context;
	long state;	// the state of the currently running task, support TASK_RUNNING
	long counter; // decreases by 1 each timer tick and when it reaches 0 another task is scheduled
	long priority; // When a new task is scheduled its priority is copied to counter
	long preempt_count; // if != 0 -> critical section, timer tick ignored, reschedule not triggered
    long id; // self add
};

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct task_struct* next);
extern void cpu_switch_to(struct task_struct* prev, struct task_struct* next);

/**
 * INIT_TASK: the one that runs kernel_main function
 * the only one task running after the kernel startup
 */
#define INIT_TASK \
/*cpu_context*/	{ {0,0,0,0,0,0,0,0,0,0,0,0,0}, \
/* state etc */	0,0,1, 0 \
}

#endif
#endif