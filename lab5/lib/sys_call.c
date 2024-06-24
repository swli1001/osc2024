#include "sys_call.h"
#include "sched.h"
#include "peripherals/mailbox.h"
#include "fork.h"
#include "buddy.h"
#include "malloc.h"
#include "./include/cpio.h"
#include "str_util.h"
#include "mini_uart.h"

#define NULL (void*)0xFFFFFFFFFFFFFFFF

/*each system call is actually a synchronous exception
svc instruction generates a synchronous exception. 
Such exceptions are handled at EL1 by the operating system.

The OS then validates all arguments, performs the requested action and
execute normal exception return, which ensures that the execution will 
resume at EL0 right after the svc instruction. 
*/

int sys_getpid(){
    return current->id;
}

unsigned sys_uartread(char buf[],unsigned size){
    for(unsigned int i=0;i<size;i++){
        buf[i]=uart_recv();
    }
    return size;
}

unsigned sys_uartwrite(const char buf[], unsigned size){
    for(int i=0;i<size;i++){
        uart_send(buf[i]);
    }
    return size;
}

int sys_exec(const char *name, char *const argv[]) {
    struct cpio_newc_header *header;
    unsigned int filesize;
    unsigned int namesize;
    unsigned int offset;
    char *filename;
    void *code_loc;

    header = DTB_LOAD_POS;
    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (str_cmp((char*)header, C_MAGIC) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (str_cmp(filename, ARCHIVE_END) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = hexstr_to_int(header->c_namesize);
        
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;

        filesize = hexstr_to_int(header->c_filesize);

        if (str_cmp_len(filename, name, namesize) == 0) {
            code_loc = ((void*)header) + offset;
            break;
        }

        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }

    void *move_loc = malloc(filesize + 4096); // an extra page for bss just in case
    if(move_loc == NULL) return -1;
    for (int i=0; i<filesize; i++) {
        ((char*)move_loc)[i] = ((char*)code_loc)[i];
    }
    preempt_disable();

    // free old program location
    struct pt_regs *p = task_pt_regs(current);
    p->pc = (unsigned long)move_loc; // move to beginning of program
    p->sp = current->stack+FRAME_SIZE;

    preempt_enable();

    return -1; // only on failure
}

int sys_fork(){
    return copy_process(0,0,0,(unsigned long)malloc(4*4096));
}

void sys_exit(int state){
    exit_process();
}

int sys_mbox_call(unsigned char ch, unsigned int *mbox) {
    unsigned int r = (((unsigned int)((unsigned long)mbox)&~0xF) | (ch&0xF));
    while(*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = r;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        if (r == *MAILBOX_READ) {
            return mbox[1]==REQUEST_SUCCEED;
        }
    }
    return 0;
}

void sys_kill(int pid){
    struct task_struct *p;
    for (int i=0;i<NR_TASKS;i++){
        if(task[i]==NULL) continue;

        p=task[i];
        if(p->id==(long)pid){
            preempt_disable();
            uart_send_string("Kill target thread\r\n");
            p->state=TASK_ZOMBIE;
            free_frame((void*)p->stack);
            preempt_enable();
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