#include "./include/thread.h"
#include "buddy.h"
#include "utils.h"
#include "mini_uart.h"
#include "shell.h"

#define NULL (void*)0xFFFFFFFFFFFFFFFF

// extern struct cpu_context *current; // always points to the currently executing task
static struct cpu_context * task[TASK_MAX_NUM];
// static int nr_tasks; // contains the number of currently running tasks in the system
static struct cpu_context *run_queue_head, *run_queue_tail;

void thread_init() {
    for (int i = 0; i < TASK_MAX_NUM; i++) {
            task[i] = NULL;
    }

    struct cpu_context *new_thread = alloc_frame(1);
    new_thread->id = 0;
    new_thread->next = NULL;
    new_thread->is_dead = 0;
    task[0] = new_thread;

    asm volatile("msr tpidr_el1, %0" : : "r"(new_thread));

    run_queue_head = NULL;
    run_queue_tail = NULL;
}

void thread_wrapper() {
    asm volatile("blr x19"); // branch to thread_func

    struct cpu_context *current_thread = get_current_thread();
    current_thread->is_dead = 1;

    schedule();

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

    struct cpu_context *new_thread = alloc_frame(1);
    new_thread->reg.x19 = (unsigned long)thread_func;
    new_thread->lr = (unsigned long)thread_wrapper;
    new_thread->sp = (unsigned long)new_thread + FRAME_SIZE;
    new_thread->next = NULL;
    new_thread->id = id;
    new_thread->is_dead = 0;
    task[id] = new_thread;

    if(run_queue_head == NULL) { run_queue_head = new_thread; } 
    else { run_queue_tail->next = new_thread; }
    run_queue_tail = new_thread;

    return id;
}

void schedule() {
    if (run_queue_head == NULL) {
        asm volatile("ldr x0, =_start");
        asm volatile("mov sp, x0");
        asm volatile("bl shell");
    }

    struct cpu_context *next_thread = run_queue_head;
    run_queue_head = next_thread->next;
    next_thread->next = NULL;

    struct cpu_context *current_thread = get_current_thread();
    if (current_thread->is_dead) { // kill_zombies()
        task[current_thread->id] = NULL;
        free_frame(current_thread);
    } else if (run_queue_head == NULL) {
        run_queue_head = current_thread;
        run_queue_tail = current_thread;
    } else {
        run_queue_tail->next = current_thread;
        run_queue_tail = current_thread;
    }
    cpu_switch_to(current_thread, next_thread);
}

void idle(void)
{
    while (1) {
        // kill_zombies(); // reclaim threads marked as DEAD
        schedule(); // switch to any other runnable thread
    }
}

/**
 * DEMO
 */
void foo(){
    for(int i = 0; i < 10; ++i) {
        // printf("Thread id: %d %d\n", current->id, i);
        uart_send_string("Thread id: ");
        uart_send_uint(get_current_thread()->id);
        uart_send(' ');
        uart_send_uint(i);
        uart_send_string("\r\n");
        delay(1000000);
        schedule();
    }
}

void thread_test() {
    for(int i = 0; i < 3; ++i) { // N should > 2
        thread_create(foo);
    }
    idle();
}