#include "uart.h"

void main(void) {

	uart_init();

	uart_write_asciiz("riscv64-pure64 started.\n");

	for (;;) {

	}
}
