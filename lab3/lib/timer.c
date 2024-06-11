#include "timer.h"
#include "mini_uart.h"
#include "str_util.h"

//#define CORE0_TIMER_IRQ_CTRL 0x40000040

void core_timer_enable() {
    asm volatile(
        "mov x0, 1\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable
        "mrs x0, cntfrq_el0\n\t"
        "msr cntp_tval_el0, x0\n\t" // set expired time
        "mov x0, 2\n\t"
        "ldr x1, =0x40000040\n\t"
        "str w0, [x1]\n\t" // unmask timer interrupt
    );
    uart_send_string("core timer enabled\r\n");
}

void set_time_out(unsigned int sec) {
    asm volatile("mrs x1, cntfrq_el0");
    asm volatile("mul x2, x1, %0" :: "r"(sec));
    asm volatile("msr cntp_tval_el0, x2");
}

/*void core_timer_disable() {
    asm volatile(
        "mov x0, 0\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable
        "mrs x0, cntfrq_el0\n\t"
        "msr cntp_tval_el0, x0\n\t" // set expired time
        "mov x0, 0\n\t"
        "ldr x1, =CORE0_TIMER_IRQ_CTRL\n\t"
        "str w0, [x1]\n\t" // unmask timer interrupt
    );
}*/

void print_seconds() {
    // the count of the timer(cntpct_el0)
    // the frequency of the timer(cntfrq_el0)
    unsigned long cnt_timer;
    unsigned long freq_timer;
    unsigned long cur_time;
    char stime[16];

    asm volatile("mrs %0, cntpct_el0" : "=r"(cnt_timer));
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq_timer));
    cur_time = cnt_timer / freq_timer;

    uart_send_string("timer interrupt: ");
    int_to_str(cur_time, stime, 10);
    uart_send_string(stime);
    uart_send_string("secs\r\n");
}

  