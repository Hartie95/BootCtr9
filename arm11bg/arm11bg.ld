OUTPUT_ARCH(arm)
ENTRY(a11Entry)
SECTIONS
{
	. = 0x1FFF4000;
	.text : {
		*(.text.start)
		*(.text)
	}
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss COMMON)
	}
	.rodata : {
		*(.rodata)
	}
	. = ALIGN(4);
	_stack = . + 0x100;
}
