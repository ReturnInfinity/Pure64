ENTRY(_start)

SECTIONS {
	. = 0x1200;
	.text : {
		*(.text);
	}
	.data : {
		*(.data);
	}
	.rodata : {
		*(.rodata);
	}
	.bss : {
		*(.bss);
	}
}
