.global _start
.extern main
.extern stack_start

.text

_start:
	/* setup the stack to start
	 * at 8MiB + 100KiB , giving it
	 * 100KiB to grow (ends at 8MiB). */
	mv sp, zero
	addi sp, sp, 0x81
	slli sp, sp, 24
	/* call the entry point
	 * in the C source code. */
	jal main

