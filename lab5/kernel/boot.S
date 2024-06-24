#include "mm.h"
#include "exception.h"

.section ".text.boot"

.globl _start
_start:
    mov x25, x0
	mrs	x0, mpidr_el1
	and x0, x0, #0xFF
	cbz	x0, master
	b 	proc_hang

proc_hang:
	wfe
	b 	proc_hang

master:
    bl  check_cur_el2
	bl 	from_el2_to_el1
    bl  check_cur_el1
	bl	set_exception_table
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl	memzero

	mov	sp, #LOW_MEMORY
	bl	kernel_main

check_cur_el2:
    mrs x0, CurrentEL
    and x0, x0, 12  // take CurrentEL[3:2]
    lsr x0, x0, 2   // logical shift right 
    sub x0, x0, 2
    cbnz x0, proc_hang
    ret

check_cur_el1:
    mrs x0, CurrentEL
    and x0, x0, 12  // take CurrentEL[3:2]
    lsr x0, x0, 2   // logical shift right 
    sub x0, x0, 1
    cbnz x0, proc_hang
    ret


set_exception_table:
    adr x0, exception_vector_table  
    msr vbar_el1, x0
    ret

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

/* After the kernel is initialized, load user programs and execute them in EL0 by eret. */
.globl from_el1_to_el0 // (prog_addr, usr_prog stack_top) 
from_el1_to_el0:    
    msr elr_el1, x0 // set to prog start addr
    msr sp_el0, x1 // set user prog sp to a proper pos by setting sp_el0
    mov x1, 0x340  // 11_0100_0000 only enabled IRQ (timer & uart) interrupts from el1
    msr spsr_el1, x1
    eret