.global _start

.text

_start:
	/* setup the stack to start
	 * at 8GiB + 16MiB , giving it
	 * 16MiB to grow (ends at 8GiB). */
	mv sp, zero
	addi sp, sp, 0x81
	slli sp, sp, 24
	/* Skip the nothingness and go to
	 * the next sector. */
	j next_sector
	/* Pad bytes until the end of the
	 * sector. */
.space 510-(.-_start)
	/* Add the historical boot signature. */
.byte 0xaa
.byte 0x55
	/* The beginning of the next sector is here.
	 * There may either be a kernel or a kernel
	 * loader put at this location. */
next_sector:
