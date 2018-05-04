#include "uart.h"

#ifdef PURE64_BOARD_SIFIVE_U500
#include "sifive_u500.h"
#else
#include "qemu_virt.h"
#endif

/* This UART driver was implemented
 * by referencing the Freedom E310 manual
 * at https://www.sifive.com/documentation/chips/freedom-e310-g000-manual/ */

typedef unsigned int pure64_uint32;

void uart_init(void) {

	pure64_uint32 transmit_ctrl = 0x00;
	/* Set transmit enable */
	transmit_ctrl |= 0x01;
	/* By default, the transmit control
	 * register uses one stop bit, so
	 * well just go with that. */

	/* Assign the values in the transmit
	 * control register */
	volatile pure64_uint32 *uart0_addr = (volatile pure64_uint32 *) UART0;
	/* set transmit control register */
	uart0_addr[2] = transmit_ctrl;
}

void uart_putc(char c) {

	volatile pure64_uint32 *uart0_addr = (volatile pure64_uint32 *) UART0;

	/* Ensure that the FIFO isn't full. */
	while (uart0_addr[0] & 0x80000000) {
		/* Do nothing */
	}

	uart0_addr[0] = c;
}

void uart_write_asciiz(const char *msg) {

	for (unsigned int i = 0; msg[i] != 0; i++) {
		uart_putc(msg[i]);
	}
}
