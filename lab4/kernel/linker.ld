__StackSize = 0x100000;

SECTIONS
{
	. = 0x80000;
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }

	. = ALIGN(0x8);

	bss_begin = .;
	.bss : { *(.bss*) }
	bss_end = .;

	. = ALIGN(0x8);

	__end__ = .;
	.heap : { *(.heap*) }
	. = 0x120000;
	__HeapLimit = .;

	. = . + __StackSize;
    .stack : {
        . = ALIGN(8);
        _stack_top = .;
    }
    __all_end = .;
} 

