/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "string.h"

void pure64_memset(void *dst, int value, unsigned long int size) {

	unsigned char *dst8;
	unsigned long int i;

	dst8 = (unsigned char *) dst;

	for (i = 0; i < size; i++) {
		dst8[i] = (unsigned char) value;
	}
}

void pure64_memcpy(void *dst, const void *src, unsigned long int size) {

	unsigned char *dst8;
	const unsigned char *src8;
	unsigned long int i;

	dst8 = (unsigned char *) dst;
	src8 = (const unsigned char *) src;

	for (i = 0; i < size; i++) {
		dst8[i] = src8[i];
	}
}

unsigned long int pure64_strlen(const char *str) {

	unsigned long int i = 0;

	while (str[i] != 0)
		i++;

	return i;
}
