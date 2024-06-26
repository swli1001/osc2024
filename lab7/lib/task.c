#include "task.h"
#include "malloc.h"
#include "buddy.h"
#include "list.h"
#include "mini_uart.h"
#include "timer.h"
#include "exception.h"
#include "str_util.h"

#define NULL 0

struct taskControlBlock tasks[MAX_TASKS];
int taskCount;
struct taskControlBlock *currentTask;
int preemptable = 1;

struct list readylist;
struct listItem taskListItem[MAX_TASKS];

int getCurrentPid() { return currentTask->pid; }

struct taskControlBlock* addTask(void (*func)(), int priority) {
    int freeTaskIdx = -1;
    for (int i = 1; i < MAX_TASKS; i++) {
      if (tasks[i].state == eFree) {
        freeTaskIdx = i;
        break;
      }
    }
    if (freeTaskIdx == -1) {
      uart_send_string("[ERROR] exceed max number of task\r\n");
      return NULL;
    }

    taskCount++;
    struct taskControlBlock *tsk = &tasks[freeTaskIdx];
    struct listItem *elm = &taskListItem[freeTaskIdx];
    tsk->pid = freeTaskIdx;
    tsk->priority = priority;
    tsk->state = eReady;
    tsk->userStackPage = alloc_frame(1);
    tsk->kernelStackPage = alloc_frame(1);
    tsk->exePage = (void*)NULL;
    tsk->exePage_num = 0;
    tsk->regs.lr = (unsigned long)func;
    tsk->regs.sp = (unsigned long)tsk->kernelStackPage + FRAME_SIZE;

    elm->size = sizeof(struct taskControlBlock);
    elm->data = tsk;
    listAppend(&readylist, elm);
    return tsk;
}

static void killZombies() {
    disable_preempt();
    for (int i = 1; i < MAX_TASKS; i++) {
      if (tasks[i].state == eTerminated) {
        uart_send_string("[Task] Killing ");
        uart_send_uint(i);
        uart_send_string("\r\n");
        if (tasks[i].kernelStackPage != NULL) {
          free_frame(tasks[i].kernelStackPage);
        }        
        if (tasks[i].userStackPage != NULL) {
          free_frame(tasks[i].userStackPage);
        }
        if (tasks[i].exePage != NULL) {
          free_frame(tasks[i].exePage);
        }
        memset(&tasks[i], 0, sizeof(tasks[i]));
      }
    }
    enable_preempt();
}

static void idle() {
    preemptable = 1;
    
    uart_send_string("[Task] Start Scheduler\n");
    while (1) {
      killZombies();
      schedule();
    }
}

int initIdleTask() {
    taskCount++;
    tasks[0].pid = 0;
    tasks[0].priority = MAX_PRIORITY-1;
    tasks[0].state = eRunning;
    tasks[0].kernelStackPage = alloc_frame(1);
    tasks[0].userStackPage = NULL;
    tasks[0].regs.sp = (unsigned long)tasks[0].kernelStackPage + FRAME_SIZE;
    tasks[0].regs.lr = (unsigned long)&idle;
    taskListItem[0].data = &tasks[0];
    taskListItem[0].size = sizeof(tasks[0]);
    currentTask = &tasks[0];
    return currentTask->pid;
}

void startScheduler() {
    initIdleTask();
    
    unsigned long regsp = tasks[0].regs.sp;
    unsigned long reglr = tasks[0].regs.lr;
    asm volatile( "mov lr, %0" : : "r"(reglr) );
    asm volatile( "mov sp, %0" : : "r"(regsp) );
    asm volatile( "ret" );
}

void schedule() {
    // critical section begin
    preemptable = 0;
    if(readylist.itemCount == 0) {
        uart_send_string("readylist empty\r\n");
        preemptable = 1;
        asm volatile( "ldr x0, =_start" );
        asm volatile( "mov sp, x0" );
        asm volatile( "bl shell" );
    }
    struct listItem *fstItem = readylist.first;
    struct taskControlBlock *newTsk = fstItem->data;
    struct taskControlBlock *oldTsk = currentTask;
    newTsk->state = eRunning;
    listRemoveItem(&readylist, fstItem);
    if (oldTsk->state == eRunning) {
        oldTsk->state = eReady;
        listAppend(&readylist, &taskListItem[oldTsk->pid]);
    }
    currentTask = newTsk;
    context_switch(oldTsk, newTsk);
    preemptable = 1;
}

void startInEL0(unsigned long pc) {
    disable_irq();
    currentTask->regs.spsr_el1 = 0x340;
    currentTask->regs.esr_el1 = pc;
    currentTask->userStackPage = alloc_frame(4);
    unsigned long spel0 = (unsigned long)currentTask->userStackPage + 4*FRAME_SIZE;
    asm volatile( "msr spsr_el1, %0" : : "r"(currentTask->regs.spsr_el1) );
    asm volatile( "msr elr_el1, %0" : : "r"(currentTask->regs.esr_el1) );
    asm volatile( "msr sp_el0, %0" : : "r"(spel0) );
    asm volatile( "mov sp, %0" : : "r"(currentTask->kernelStackPage + FRAME_SIZE) );
    asm volatile( "eret" );
}

int syscall_getpid() { return getCurrentPid(); }

void syscall_exit() {
    currentTask->state = eTerminated;
    schedule();
}

void enable_preempt() { preemptable = 1; }
void disable_preempt() { preemptable = 0; }
