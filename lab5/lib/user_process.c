#include "user_process.h"
#include "buddy.h"
#include "utils.h"
#include "mini_uart.h"

void create_and_execute(void *prog_ptr) {
    process_create();
    move_to_user_mode();
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