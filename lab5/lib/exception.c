#include "exception.h"
#include "mini_uart.h"
#include "timer.h"
#include "utils.h"
#include "task.h"

extern void uart_irq_handler();

void sync(void) {
    unsigned long esr_el1;
    unsigned long elr_el1;
    unsigned long spsr_el1;

    // Get EL1 registers
    /**
     * ESR_ELx:  save the cause of Sync / SError exception
     * ELR_ELx:  exception return address
     * SPSR_ELx: DAIF, should be 0x340
     */
    asm volatile("mrs %0,  esr_el1\n\t" : "=r"(esr_el1));
    asm volatile("mrs %0,  elr_el1\n\t" : "=r"(elr_el1));
    asm volatile("mrs %0, spsr_el1\n\t" : "=r"(spsr_el1));

    uart_send_string("Synchronous\r\n");

    uart_send_string("ESR_EL1 : 0x");
    uart_send_hex(esr_el1);
    uart_send_string("\r\n");
    uart_send_string("ELR_EL1 : 0x");
    uart_send_hex(elr_el1);
    uart_send_string("\r\n");
    uart_send_string("SPSR_EL1: 0x");
    uart_send_hex(spsr_el1);
    uart_send_string("\r\n\r\n");

    while(1) {} // for lab5, sync() should not be executed
}

void irq_from_el0(void) {
    //uart_send_string("IRQ from EL0\r\n");
    disable_el1_interrupt();
    unsigned int is_timer_irq, is_uart_irq;
    // judge interrupt source is from timer or uart
    is_timer_irq = get32(CORE0_IRQ_SOURCE) & (1 << 1);
    is_uart_irq  = get32(CORE0_IRQ_SOURCE) & (1 << 8);
    if(is_timer_irq) {
        // print_seconds();
        // set_time_out_cmp(2);
        // timer_expire_handler();
        timerInterruptHandler();
    }
    else if(is_uart_irq) {
        if(get32(IRQ_PENDING_1) & (1<<29)) {
            uart_irq_handler();
        }
    }
    else {
        uart_send_hex(get32(CORE0_IRQ_SOURCE));
        uart_send_string("\r\n");
        while(1) { asm volatile("nop"); }
    }
    enable_el1_interrupt();
}

void irq_from_el1(void) {
    //uart_send_string("IRQ from EL1\r\n");
    disable_el1_interrupt();
    unsigned int is_timer_irq, is_uart_irq;
    // judge interrupt source is from timer or uart
    is_timer_irq = get32(CORE0_IRQ_SOURCE) & (1 << 1);
    is_uart_irq  = get32(CORE0_IRQ_SOURCE) & (1 << 8);

    if(is_timer_irq) { 
        // timer_expire_handler(); 
        timerInterruptHandler();
    }
    else if(is_uart_irq) {  }
    enable_el1_interrupt();
}

void undefined(void) {
    uart_send_string("Undefined\r\n");
    while(1) {};
}

void enable_aux_interrupt() { // keyboard interrupt
	unsigned int reg;
	reg = get32(ENABLE_IRQS_1);
	reg = reg | (1 << 29);
	put32(ENABLE_IRQS_1, reg);
}

void disable_aux_interrupt() {
	unsigned int reg;
	reg = get32(DISABLE_IRQS_1);
	reg = reg | (1 << 29);
	put32(DISABLE_IRQS_1, reg);
}

/**
 * mask / unmask interrupts
 * D: Masks debug exceptions (special type of synchronous exceptions)
 * A: Masks SErrors
 * I: Masks IRQs
 * F: Masks FIQs
 */

void enable_el1_interrupt() {
    asm volatile("msr DAIFClr, 0xf");
}

void disable_el1_interrupt() {
    asm volatile("msr DAIFSet, 0xf");
}

void enable_irq() {
  asm volatile("msr DAIFClr, #2");
}

void disable_irq() {
  asm volatile("msr DAIFSet, #2");
}

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1t",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32",

	"SYNC_ERROR",
	"SYSCALL_ERROR"	
};

void show_invalid_entry_message(int type, unsigned long esr, unsigned long addr) {
    uart_send_string((const char*)entry_error_messages[type]);
    uart_send_string(", ESR: 0x");
    uart_send_hex(esr);
    uart_send_string(", address: 0x"); // ELR
    uart_send_hex(addr);
    uart_send_string("\r\n");
}
