.global _start
.extern main
.extern stack_start

.text

_start:
	/* setup the stack to start
	 * at 8GiB + 16MiB , giving it
	 * 16MiB to grow (ends at 8GiB). */
	mv sp, zero
	addi sp, sp, 0x81
	slli sp, sp, 24
	/* call the entry point
	 * in the C source code. */
	jal main

