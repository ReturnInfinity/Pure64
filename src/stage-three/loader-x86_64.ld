ENTRY(_start)

SECTIONS {
	. = 0x60000;
	.text : {
		*(.text._start);
		*(.text);
	}
}
