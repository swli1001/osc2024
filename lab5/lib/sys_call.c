#include "sys_call.h"
// #include "sched.h"
#include "peripherals/mailbox.h"
#include "fork.h"
#include "buddy.h"
#include "malloc.h"
#include "./include/cpio.h"
#include "str_util.h"
#include "mini_uart.h"
#include "task.h"
#include "fork.h"

#define NULL (void*)0xFFFFFFFFFFFFFFFF

/**
 * svc instruction generates a synchronous exception. 
 * Such exceptions are handled at EL1 by the operating system.
 * The OS then validates all arguments, performs the requested action and
 * execute normal exception return, which ensures that the execution will 
 * resume at EL0 right after the svc instruction. 
 */

int sys_getpid(){
    return syscall_getpid();
}

unsigned sys_uartread(char buf[],unsigned size){
    for(unsigned int i = 0; i < size; i++){
        buf[i] = uart_recv();
    }
    return size;
}

unsigned sys_uartwrite(const char buf[], unsigned size){
    for(int i = 0; i < size; i++){
        uart_send(buf[i]);
    }
    return size;
}

int sys_exec(const char *name, char *const argv[]) {
    struct cpio_newc_header * buf = DTB_LOAD_POS;
    char *filename;
    int namesize, filesize;
    int blocksize;
    void *code_loc;

    while(1) {
        filename = (char*)buf + HEADER_SIZE;
        if(str_cmp(filename, ARCHIVE_END) == 0) { break; }

        filesize = hexstr_to_int(buf->c_filesize);
        namesize = hexstr_to_int(buf->c_namesize);

        blocksize = HEADER_SIZE + namesize;
        if (blocksize % 4 != 0) { blocksize += 4 - (blocksize % 4); }

        if(str_cmp_len(filename, name, namesize) == 0) {
            code_loc = ((void*)buf) + blocksize;
            break;
        }

        if (filesize % 4 != 0) { filesize += 4 - (filesize % 4); }
        blocksize += filesize;

        buf = (void*)buf + blocksize;
    }

    int page_cnt = filesize / FRAME_SIZE;
    if(filesize % FRAME_SIZE) { page_cnt++; }

    currentTask->exePage = alloc_frame(page_cnt);
    currentTask->exePage_num = page_cnt;
    if(currentTask->exePage == (void*)0) return -1;
    for (int i = 0; i < filesize; i++) {
        ((char*)currentTask->exePage)[i] = ((char*)code_loc)[i];
    }

    disable_preempt();
    struct el0_regs *trap_frame = (struct el0_regs*)(currentTask->kernelStackPage
                                    + FRAME_SIZE - sizeof(struct el0_regs));
    trap_frame->elr_el1 = (unsigned long)currentTask->exePage;
    trap_frame->sp_el0 = (unsigned long)currentTask->userStackPage + 4*FRAME_SIZE;
    enable_preempt();

    return -1; // only on failure
}

int sys_fork(){
    return syscall_fork();
}

void sys_exit(int state){
    return syscall_exit();
}

int sys_mbox_call(unsigned char ch, unsigned int *mbox) {
    unsigned int r = (((unsigned int)((unsigned long)mbox)&~0xF) | (ch&0xF));
    while(*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = r;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        if (r == *MAILBOX_READ) {
            return mbox[1] == REQUEST_SUCCEED;
        }
    }
    return 0;
}

void sys_kill(int pid){
    struct task_struct *p;
    for (int i = 0; i < MAX_TASKS; i++){
        if(tasks[i].state == eFree) continue;

        if(tasks[i].pid == (unsigned long)pid){
            disable_preempt();
            uart_send_string("Kill target thread\r\n");
            tasks[i].state = eTerminated;
            enable_preempt();
            break;
        }
    }
}

void * const sys_call_table[] =
{
    sys_getpid,
    sys_uartread,
    sys_uartwrite,
    sys_exec,
    sys_fork,
    sys_exit,
    sys_mbox_call,
    sys_kill
};