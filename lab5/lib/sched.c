#include "./include/sched.h"
#include "exception.h"
#include "mini_uart.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct *task[TASK_MAX_NUM] = {&(init_task), };
int nr_tasks = 1;

void preempt_disable() {
    current->preempt_count++;
}

void preempt_enable() {
    current->preempt_count--;
}

void _schedule() {
    // preempt_disable(); // original pos
    int next, c;
    struct task_struct *p;
    while (1) {
        c = -1;
        next = 0;
        /**
         * find a task in TASK_RUNNING state with the maximum counter
         */
        for (int i=0; i<TASK_MAX_NUM; i++) {
            p = task[i];
            if (p && p->state == TASK_RUNNING && p->counter > c) {
                c = p->counter;
                next = i;
            }
        }
        if (c) {
            break; // switch to the task if its counter > 0
        }
        /**
         * all tasks are waiting for an interrupt -> enable interrupt in timer_tick()
         * increase the counter of all tasks
         * more iterations -> more counter increased, but can never get > 2 * priority
         */
        for (int i=0; i<TASK_MAX_NUM; i++) {
            p = task[i];
            if (p)
                p->counter = (p->counter >> 1) + p->priority;
        }
    }
    preempt_disable();
    switch_to(task[next]);
    preempt_enable();

}

void schedule() {
    current->counter = 0;
    _schedule();
}

void switch_to(struct task_struct *next) {
    if (current == next)
        return;
    struct task_struct *prev = current;
    current = next;
    cpu_switch_to(prev, next);
}

void schedule_tail() {
    preempt_enable();
}

void timer_tick() {
    
    --current->counter;
    if (current->counter > 0 || current->preempt_count > 0)
        return;

    current->counter = 0;
    /**
     * interrupt happen -> can change the state of the task 
     * 
     */
    enable_el1_interrupt(); 
    _schedule();
    disable_el1_interrupt();
}