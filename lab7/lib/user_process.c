#include "user_process.h"
#include "buddy.h"
#include "utils.h"
#include "mini_uart.h"

void forkProcedureForChild() {
    enable_preempt();
    struct el0_regs *trap_frame = (struct el0_regs*)(currentTask->kernelStackPage
                                                    + FRAME_SIZE - sizeof(struct el0_regs));
    trap_frame->general_purpose[0] = 0;
    disable_irq();
    asm volatile("mov sp, %0\n\t"
                "b exit_kernel" :: "r"(trap_frame));
}

int copyCurrentTask() {
    disable_preempt();
    struct taskControlBlock *newTask = addTask(forkProcedureForChild, currentTask->priority);
    if (newTask == NULL) return -1;
    // copy user stack and set newTask's sp_el0 before it eret
    uint64_t spel0, elr_el1;

    asm volatile("mrs %0, sp_el0\n\t"
                "mrs %1, elr_el1" : "=r"(spel0), "=r"(elr_el1));

    uint64_t sp_offset = spel0 - (uint64_t)currentTask->userStackPage;
    newTask->userStackPage = getContFreePage(4, &currentTask->userStackExp);

    memcpy((void*)newTask->userStackPage + sp_offset,
            (void*)currentTask->userStackPage + sp_offset,
            (4*FRAME_SIZE)-sp_offset);

    newTask->regs.spsr_el1 = 0x340;
    newTask->regs.elr_el1 = elr_el1;
    newTask->regs.sp = (uint64_t)newTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs);
    newTask->regs.sp_el0 = (uint64_t)newTask->userStackPage + sp_offset;

    // copy trapframe
    struct el0_regs *curTrapFrame = (struct el0_regs*)(currentTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
    struct el0_regs *newTrapFrame = (struct el0_regs*)(newTask->kernelStackPage + FRAME_SIZE - sizeof(struct el0_regs));
    memcpy(newTrapFrame, curTrapFrame, sizeof(struct el0_regs));

    enable_preempt();
    return newTask->pid;
}

int syscall_fork() {
  return copyCurrentTask();
}

void startInEL0(uint64_t pc) {
  disable_irq();
  currentTask->regs.spsr_el1 = 0x340; // enable irq (disable 3C0)
  currentTask->regs.esr_el1 = pc;
  currentTask->userStackPage = getContFreePage(4, &currentTask->userStackExp);
  uint64_t spel0 = (uint64_t)currentTask->userStackPage + 4*FRAME_SIZE;
  kprintf("[K] Move process no.%d to userspace.\n", currentTask->pid);
  asm volatile("msr spsr_el1, %0\n\t"
               "msr elr_el1, %1\n\t"
               "msr sp_el0, %2\n\t"
               "mov sp, %3\n\t" // clear kernel stack
               "eret"
               ::
                "r" (currentTask->regs.spsr_el1),
                "r" (currentTask->regs.esr_el1),
                "r" (spel0),
                "r" (currentTask->kernelStackPage + FRAME_SIZE));
}

/**
 * Basic 2 DEMO
 */

void fork_test(){
    printf("\nFork Test, pid %d\n", get_pid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit(); // exit(0)
    }
    else {
        printf("parent here, pid %d, child %d\n", get_pid(), ret);
    }
    // exit(); ?? exit(0)
}

void foo_user_02(void)
{
        uart_send_string("FOO 2 IS HERE\r\n");
        exit(0);
}

void foo_user_01(void)
{
        uart_send_string("FOO 1 IS HERE\r\n");
        uart_send_int(getpid());
        uart_endl();
        int ret = 0;
        if ((ret = fork()) == 0) {
                // child
                uart_send_string("child is here: ");
                uart_send_int(getpid());
                uart_endl();
        } else {
                // parent
                uart_send_string("parent is here, child = ");
                uart_send_int(ret);
                uart_endl();
        }
        int ret2 = fork();
        kill(ret2);
        uart_send_string("FOO 1 DONE\r\n");
        exit(0);
}

void demo_user_proc(void)
{
        // create_and_execute(foo_user_01);
        create_and_execute(fork_test);
}