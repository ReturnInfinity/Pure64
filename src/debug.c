/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "debug.h"

static void outb(unsigned short int port, unsigned char value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static unsigned char inb(unsigned short int port) {
	unsigned char value;
	asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

#define COM1 0x03f8

void serial_putc(char c) {
	for (;;) {
		/* check if transmit is ready */
		if (inb(COM1 + 5) & 0x20)
			break;
	}
	outb(COM1, c);
}

void debug(const char *fmt, ...) {
	while (*fmt) {
		serial_putc(*fmt);
		fmt++;
	}
}
