_heap_stack_size = 1M;

SECTIONS
{
    /* bootloader 0x60000, kernel 0x80000 */
    . = 0x80000;
    _start = .;
    .text : { *(.text.boot) *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : {
        _bss_top = .;
        *(.bss) 
    }
    _bss_size = SIZEOF(.bss) >> 3;
    .heap : {
        . = ALIGN(8);
        _heap_top = .; /* Start of the heap */
        . += _heap_stack_size; /* Allocate heap space */
        _heap_end = .; /* Mark the end of the heap */
    } 
    .stack : {
        . = ALIGN(8);
        _stack_top = .; /* Start of the stack */
        /* Define stack size or grow down dynamically */
    }
    _end = .; /* End of all sections */
}
