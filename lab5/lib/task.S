 .global context_switch
context_switch:
        add x9, x0, xzr
        mov x10, sp
        mrs x11, spsr_el1
        mrs x12, elr_el1
        mrs x13, esr_el1
        mrs x14, sp_el0
        stp x19, x20, [x9, 16 * 0]
        stp x21, x22, [x9, 16 * 1]
        stp x23, x24, [x9, 16 * 2]
        stp x25, x26, [x9, 16 * 3]
        stp x27, x28, [x9, 16 * 4]
        stp fp, lr, [x9, 16 * 5]
        stp x10, x11, [x9, 16 * 6]
        stp x12, x13, [x9, 16 * 7]
        str x14, [x9, 16 * 8]

        add x9, x1, xzr
        ldp x19, x20, [x9, 16 * 0]
        ldp x21, x22, [x9, 16 * 1]
        ldp x23, x24, [x9, 16 * 2]
        ldp x25, x26, [x9, 16 * 3]
        ldp x27, x28, [x9, 16 * 4]
        ldp fp, lr, [x9, 16 * 5]
        ldp x10, x11, [x9, 16 * 6]
        ldp x12, x13, [x9, 16 * 7]
        ldr x14, [x9, 16 * 8]
        mov sp, x10
        msr spsr_el1, x11
        msr elr_el1, x12
        msr esr_el1, x13
        msr sp_el0, x14
        ret