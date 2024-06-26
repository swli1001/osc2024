#ifndef	_EXCEPTION_H
#define	_EXCEPTION_H

#include "peripherals/base.h"

#ifndef __ASSEMBLER__

#define S_FRAME_SIZE    272  // size of all saved registers 

// ***************************************
// ESR_EL1, Exception Syndrome Register (EL1). Page 2431 of AArch64-Reference-Manual.
// ***************************************

#define ESR_ELx_EC_SHIFT		26
#define ESR_ELx_EC_SVC64		0x15

void sync_from_el0 ( void );
void irq_from_el0 ( void );
void irq_from_el1( void );
void undefined( void );

void enable_aux_interrupt();
void disable_aux_interrupt();

void enable_el1_interrupt();
void disable_el1_interrupt();

void enable_irq();
void disable_irq();

void show_invalid_entry_message(int type, unsigned long esr, unsigned long addr);

void undefined(void);

#endif

#endif
