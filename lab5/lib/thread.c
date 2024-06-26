#include "./include/thread.h"
#include "buddy.h"
#include "utils.h"
#include "mini_uart.h"
#include "shell.h"
#include "exception.h"

#define NULL (void*)0xFFFFFFFFFFFFFFFF

// extern struct thread *current; // always points to the currently executing task
static struct thread * task[TASK_MAX_NUM];
// static int nr_tasks; // contains the number of currently running tasks in the system
static struct thread *run_queue_head, *run_queue_tail;

#define FIRST_TASK task[0]
#define LAST_TASK task[TASK_MAX_NUM-1]

static unsigned int now_preemptable = 1;

void thread_init() {
    for (int i = 0; i < TASK_MAX_NUM; i++) {
        task[i] = NULL;
    }

    struct thread *new_thread = alloc_frame(1);
    new_thread->id = 0;
    new_thread->next = NULL;
    new_thread->state = TASK_WAITING;
    task[0] = new_thread;

    asm volatile( "msr tpidr_el1, %0" : : "r"(new_thread) );

    run_queue_head = NULL;
    run_queue_tail = NULL;
}

void thread_wrapper() {
    asm volatile( "blr x19" ); // branch to thread_func

    struct thread *current_thread = get_current_thread();
    current_thread->state = TASK_DEAD;

    thread_schedule();

    while (1) {}
}

int thread_create(void (*thread_func)(void)) {
    int id;
    for (id = 0; id < TASK_MAX_NUM && task[id] != NULL; id++);
    if (id >= TASK_MAX_NUM) {
        uart_send_string("[ERROR] exceed max number of thread\r\n");
        while (1) {}
        return -1;
    }

    struct thread *new_thread = alloc_frame(1);
    new_thread->context.reg.x19 = (unsigned long)thread_func;
    new_thread->context.lr = (unsigned long)thread_wrapper;
    new_thread->user_stack = alloc_frame(1);
    new_thread->kernel_stack = alloc_frame(1);  
    new_thread->context.sp = (unsigned long)new_thread->kernel_stack + FRAME_SIZE;      
    new_thread->id = id;
    new_thread->state = TASK_WAITING;
    new_thread->prev = run_queue_tail;
    new_thread->next = NULL;
    task[id] = new_thread;

    if(run_queue_head == NULL) { run_queue_head = new_thread; } 
    else { run_queue_tail->next = new_thread; }
    run_queue_tail = new_thread;

    return id;
}

/**
 * SCHEDULE
 */
void thread_enable_preempt() { now_preemptable = 1; }
void thread_disable_preempt() { now_preemptable = 0; }

void thread_schedule() {
    uart_send_string("schedule\r\n");
    thread_disable_preempt();

    if (run_queue_head == NULL) {
        asm volatile( "ldr x0, =_start" );
        asm volatile( "mov sp, x0" );
        asm volatile( "bl shell" );
    }

    struct thread *next_thread = run_queue_head;
    run_queue_head = next_thread->next;
    next_thread->next = NULL;

    struct thread *current_thread = get_current_thread();
    if (current_thread->state == TASK_DEAD) { // kill_zombies()
        task[current_thread->id] = NULL;
        free_frame(current_thread);
    } else if (run_queue_head == NULL) {
        run_queue_head = current_thread;
        run_queue_tail = current_thread;
    } else {
        run_queue_tail->next = current_thread;
        run_queue_tail = current_thread;
    }
    thread_context_switch(current_thread, next_thread);

    thread_enable_preempt();
}

void idle(void)
{
    while (1) {
        // kill_zombies(); // reclaim threads marked as DEAD
        thread_schedule(); // switch to any other runnable thread
    }
}

/**
 * DEMO
 */
void thread_foo(){
    for(int i = 0; i < 10; ++i) {
        // printf("Thread id: %d %d\n", current->id, i);
        uart_send_string("Thread id: ");
        uart_send_uint(get_current_thread()->id);
        uart_send(' ');
        uart_send_uint(i);
        uart_send_string("\r\n");
        delay(1000000);
        thread_schedule();
    }
}

void thread_test() {
    for(int i = 0; i < 3; ++i) { // N should > 2
        thread_create(thread_foo);
    }
    idle();
}