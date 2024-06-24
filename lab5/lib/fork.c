#include "fork.h"
#include "malloc.h"
#include "mini_uart.h"
#include "sched.h"
#include "entry.h"
#include "buddy.h"
#include "mm.h"

#define NULL (void*)0xFFFFFFFFFFFFFFFF

// supports cloning user threads as well as kernel threads
int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg, unsigned long stack){
    preempt_disable();
    struct task_struct *p;

    p=(struct task_struct *)malloc(FRAME_SIZE);
    if(p==NULL) return -1;

    struct pt_regs *childregs = task_pt_regs(p);
    memzero((unsigned long)childregs, sizeof(struct pt_regs));
    memzero((unsigned long)&p->cpu_context, sizeof(struct cpu_context));

    if (clone_flags & PF_KTHREAD) {
        //creating a new kernel thread
        p->cpu_context.x19 = fn;
        p->cpu_context.x20 = arg;
    } else {
        //cloning a user thread
        struct pt_regs *cur_regs = task_pt_regs(current);       //returns pt_regs area at the top of the kernel stack
        // *childregs = *cur_regs; (object file generates memcpy)
        // therefore the for loop is used below
        for(int i=0; i<sizeof(struct pt_regs); i++) {
            ((char*)childregs)[i] = ((char*)cur_regs)[i];
        }
        /*In the second line current processor state is copied to the new task's state. 
        x0 in the new state is set to 0, because x0 will be interpreted by the caller as a return value of the syscall. 
        */
        childregs->regs[0] = 0; // return value 0
        /*Next sp for the new task is set to point to the top of the new user stack page. 
        We also save the pointer to the stack page in order to do a cleanup after the task finishes.*/
        childregs->sp = stack + FRAME_SIZE;
        p->stack = stack;
    }

    p->flags = clone_flags;
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority;
    p->preempt_count = 1;
    
    p->cpu_context.pc = (unsigned long)ret_from_fork;
    p->cpu_context.sp = (unsigned long)childregs;

    int pid = nr_tasks++;
    task[pid] = p;
    p->id = pid;
    preempt_enable();

    return pid;
}

int move_to_user_mode(unsigned long pc){
    struct pt_regs *regs=task_pt_regs(current);     //current process state
    memzero((unsigned long)regs, sizeof(*regs));
    regs->pc=pc;
    regs->pstate=PSR_MODE_EL0t;
    unsigned long stack =(unsigned long)malloc(FRAME_SIZE);  //allocate new user stack
    
    if(stack==NULL) return -1;      //no enough space, creating failed
    /*allocates a new page for the user stack and sets sp field to point to the top of this page.*/
    regs->sp = stack + FRAME_SIZE;
    current->stack=stack;
    return 0;

}

// used to calculate the location of the pt_regs area
//Because of the way we initialized the current kernel thread, we are sure that after it finished sp will point right before the pt_regs area
struct pt_regs *task_pt_regs(struct task_struct *tsk) {

    unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
    return (struct pt_regs *)p;

}


void new_user_process(unsigned long func){
    // printf("Kernel process started, moving to user mode.\n");
	uart_send_string("Kernel process started, moving to user mode.\r\n");
    int err=move_to_user_mode(func);
    if(err<0){
        // printf("Error while moving process to user mode\n\r");
		uart_send_string("Error while moving process to user mode\r\n");
    }
}

/**
 * BASIC 2 DEMO
 */

void foo_user_02(void)
{
    uart_send_string("FOO 2 IS HERE\r\n");
    exit(0);
}

void foo_user_01(void)
{
    
    
    uart_send_string("FOO 1 IS HERE, pid: ");
    uart_send_uint(getpid());
    uart_send_string("\r\n");
    int ret = 0;
    if ((ret = fork()) == 0) {
            // child
            uart_send_string("child is here: ");
            uart_send_uint(getpid());
            uart_send_string("\r\n");
    } else {
            // parent
            uart_send_string("parent is here, child = ");
            uart_send_uint(ret);
            uart_send_string("\r\n");
    }
    int ret2 = fork();
    kill(ret2);
    uart_send_string("FOO 1 DONE\r\n");
    exit(0);
}

void fork_test(){
    // printf("\nFork Test, pid %d\n", get_pid());

    uart_send_string("\r\nFork Test, pid ");
    uart_send_uint(getpid());
    uart_send_string("\r\n");
    unsigned int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        uart_send_string("first child pid: ");
        uart_send_uint(getpid());
        uart_send_string(", cnt: ");
        uart_send_uint(cnt);
        uart_send_string(", ptr: 0x");
        uart_send_hex((unsigned long)&cnt);
        uart_send_string(", sp: 0x");
        uart_send_hex((unsigned long)cur_sp);
        uart_send_string("\r\n");
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
            uart_send_string("first child pid: ");
            uart_send_uint(getpid());
            uart_send_string(", cnt: ");
            uart_send_uint(cnt);
            uart_send_string(", ptr: 0x");
            uart_send_hex((unsigned long)&cnt);
            uart_send_string(", sp: 0x");
            uart_send_hex((unsigned long)cur_sp);
            uart_send_string("\r\n");
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                // printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
                uart_send_string("second child pid: ");
                uart_send_uint(getpid());
                uart_send_string(", cnt: ");
                uart_send_uint(cnt);
                uart_send_string(", ptr: 0x");
                uart_send_hex((unsigned long)&cnt);
                uart_send_string(", sp: 0x");
                uart_send_hex((unsigned long)cur_sp);
                uart_send_string("\r\n");
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        // printf("parent here, pid %d, child %d\n", get_pid(), ret);
        uart_send_string("parent here, pid: ");
        uart_send_uint(getpid());
        uart_send_string(", child: ");
        uart_send_uint(ret);
        uart_send_string("\r\n");
    }
    exit(); // ?
}
