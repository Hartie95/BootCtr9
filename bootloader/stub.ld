ENTRY(_start)
SECTIONS
{
    . = 0x24F00000;
    .text.start : { *(.text.start) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    .rodata : { *(.rodata) }
    . = ALIGN(4);
    __bss_end__ = ABSOLUTE(.);
	__end__ = ABSOLUTE(.);
}


		