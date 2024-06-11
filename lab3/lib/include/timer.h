#ifndef __TIMER_H
#define __TIMER_H

/*

cntpct_el0: The timer’s current count.

cntp_cval_el0: A compared timer count. If cntpct_el0 >= cntp_cval_el0, interrupt the CPU core.

cntp_tval_el0: (cntp_cval_el0 - cntpct_el0). You can use it to set an expired timer after the 
                current timer count.

*/
#ifndef __ASSEMBLER__

void core_timer_enable();
void set_time_out(unsigned int sec);
void print_seconds();

#endif

#endif