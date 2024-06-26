#ifndef TASK_H
#define TASK_H

#define MAX_TASKS 64
#define MAX_PRIORITY 99

typedef enum { eFree=0, eRunning, eReady, eBlocked, eTerminated} eTaskState;

struct el1_regs {
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
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long esr_el1;
    unsigned long sp_el0;
};

struct el0_regs { // trap frame, placed kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs)
    unsigned long general_purpose[31];
    unsigned long elr_el1;
    unsigned long spsr_el1;
    unsigned long sp_el0;
};

struct taskControlBlock {
    struct el1_regs regs;
    unsigned int pid;
    unsigned int priority;
    eTaskState state;
    void* kernelStackPage;
    void* userStackPage;
    void* exePage; // store .img file content
    unsigned int exePage_num;
};

extern struct taskControlBlock tasks[MAX_TASKS];
extern struct taskControlBlock *currentTask;
extern int preemptable;

int getCurrentPid();
struct taskControlBlock* addTask(void (*func)(), int priority);

void schedule();
void startScheduler();
void reset_timer();
void timerInterruptHandler();
void startInEL0(unsigned long pc);


int syscall_getpid();
void syscall_exit();
void disable_preempt();
void enable_preempt();

extern void context_switch(struct taskControlBlock *old_tcb,
                           struct taskControlBlock *new_tcb);


#endif