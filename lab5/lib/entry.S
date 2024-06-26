#include "entry.h"
#include "exception.h"
# successful

.macro save_all
        sub sp, sp, 34 * 8
        stp x0, x1, [sp ,16 * 0]
        stp x2, x3, [sp ,16 * 1]
        stp x4, x5, [sp ,16 * 2]
        stp x6, x7, [sp ,16 * 3]
        stp x8, x9, [sp ,16 * 4]
        stp x10, x11, [sp ,16 * 5]
        stp x12, x13, [sp ,16 * 6]
        stp x14, x15, [sp ,16 * 7]
        stp x16, x17, [sp ,16 * 8]
        stp x18, x19, [sp ,16 * 9]
        stp x20, x21, [sp ,16 * 10]
        stp x22, x23, [sp ,16 * 11]
        stp x24, x25, [sp ,16 * 12]
        stp x26, x27, [sp ,16 * 13]
        stp x28, x29, [sp ,16 * 14]
        mrs x21, elr_el1
        mrs x22, spsr_el1
        stp x30, x21, [sp, 16 * 15]
        str x22, [sp, 16 * 16]
.endm

// load general registers from stack
.macro load_all
        ldp x21, x22, [sp, 31 * 8]
        msr elr_el1, x21
        msr spsr_el1, x22
        ldp x0, x1, [sp ,16 * 0]
        ldp x2, x3, [sp ,16 * 1]
        ldp x4, x5, [sp ,16 * 2]
        ldp x6, x7, [sp ,16 * 3]
        ldp x8, x9, [sp ,16 * 4]
        ldp x10, x11, [sp ,16 * 5]
        ldp x12, x13, [sp ,16 * 6]
        ldp x14, x15, [sp ,16 * 7]
        ldp x16, x17, [sp ,16 * 8]
        ldp x18, x19, [sp ,16 * 9]
        ldp x20, x21, [sp ,16 * 10]
        ldp x22, x23, [sp ,16 * 11]
        ldp x24, x25, [sp ,16 * 12]
        ldp x26, x27, [sp ,16 * 13]
        ldp x28, x29, [sp ,16 * 14]
        ldr x30, [sp, 16 * 15]
        add sp, sp, 34 * 8
.endm

invalid_exception_handler:
        save_all
        bl undefined
        load_all
        eret

el1_irq_handler:
        save_all
        bl irq_from_el1
        load_all
        eret
 
el0_irq_handler:
        save_all
        bl irq_from_el0
        load_all
        eret

syscall_handler:
        // use callee saved register
        adr x19, sys_call_table // syscall function address table
        uxtw x20, w8 // extend syscall nr to 64bit and saved in x10
        bl enable_irq
        ldr x21, [x19, x20, lsl #3] // x11 = *(syscall_vectos + syscall nr * 8) 
        blr x21
        str x0, [sp] // save return value onto the stack, overwrite the el0 context
        bl disable_irq
        load_all
        eret

el0_sync_handler:
        save_all
        mrs x25, esr_el1
        lsr x24, x25, #ESR_ELx_EC_SHIFT
        cmp x24, #ESR_ELx_EC_SVC64
        b.eq syscall_handler
        bl sync
        load_all
        eret

.global exit_kernel
exit_kernel:
        load_all
        eret

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
        b invalid_exception_handler // branch to a handler function.
        .align 7 // entry size is 0x80, .align will pad 0
        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7

        b invalid_exception_handler
        .align 7
        b el1_irq_handler
        .align 7
        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7

        b el0_sync_handler
        .align 7
        b el0_irq_handler
        .align 7
        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7

        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7
        b invalid_exception_handler
        .align 7