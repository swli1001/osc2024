#include "task.h"
#include "malloc.h"
#include "buddy.h"
#include "exception.h"
#include "fork.h"
#include "mini_uart.h"
#include "str_util.h"
#include "sys_call.h"

extern struct taskControlBlock *currentTask;

#define NULL 0

void forkProcedureForChild() {
    enable_preempt();
    struct el0_regs *trap_frame = (struct el0_regs*)(currentTask->kernelStackPage
                                                    + FRAME_SIZE - sizeof(struct el0_regs));    
    trap_frame->general_purpose[0] = 0;
    disable_el1_interrupt();
    asm volatile("mov sp, %0\n\t"
                 "b exit_kernel" :: "r"(trap_frame));
}

int copyCurrentTask() {
    disable_preempt();
    struct taskControlBlock *newTask = addTask(forkProcedureForChild, currentTask->priority);
    if (newTask == NULL) return -1;
    // copy user stack and set newTask's sp_el0 before it eret
    unsigned long spel0, elr_el1;
    
    asm volatile("mrs %0, sp_el0\n\t"
                 "mrs %1, elr_el1" : "=r"(spel0), "=r"(elr_el1));
    
    unsigned long sp_offset = spel0 - (unsigned long)currentTask->userStackPage;
    newTask->userStackPage = alloc_frame(4);
    
    memcpy((void*)newTask->userStackPage + sp_offset,
            (void*)currentTask->userStackPage + sp_offset,
            (4*FRAME_SIZE)-sp_offset);

    newTask->regs.spsr_el1 = 0x340;
    newTask->regs.elr_el1 = elr_el1;
    newTask->regs.sp = (unsigned long)newTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs);
    newTask->regs.sp_el0 = (unsigned long)newTask->userStackPage + sp_offset;

    // copy userstack
    for(int i = 0; i < FRAME_SIZE; i++) { 
        ((char*)newTask->userStackPage)[i] = ((char*)currentTask->userStackPage)[i]; 
    }
    // copy kernelstack
    for(int i = 0; i < FRAME_SIZE; i++) { 
        ((char*)newTask->kernelStackPage)[i] = ((char*)currentTask->kernelStackPage)[i]; 
    }
    // copy exePage
    if(currentTask->exePage_num != 0) {
        newTask->exePage = alloc_frame(currentTask->exePage_num);
        newTask->exePage_num = currentTask->exePage_num;
        for(int i = 0; i < currentTask->exePage_num; i++) {
            ((char*)newTask->exePage)[i] = ((char*)currentTask->exePage)[i]; 
        }
    }
    // copy trapframe
    struct el0_regs *curTrapFrame = (struct el0_regs*)(currentTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
    struct el0_regs *newTrapFrame = (struct el0_regs*)(newTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
    memcpy(newTrapFrame, curTrapFrame, sizeof(struct el0_regs));
    newTrapFrame->sp_el0 += newTask->kernelStackPage - currentTask->kernelStackPage; // newly add
    
    enable_preempt();
    curTrapFrame->general_purpose[0] = newTask->pid;
    return newTask->pid;
}

int syscall_fork() {
    return copyCurrentTask();
}

void fork_test(){
    // printf("\nFork Test, pid %d\n", get_pid());
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
    exit(); // todo: if this line is not added, will cause undefined sync exception
}