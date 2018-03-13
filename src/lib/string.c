/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/string.h>

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

int pure64_memcmp(const void *a, const void *b, unsigned long int size) {

	const unsigned char *a8 = (const unsigned char *) a;
	const unsigned char *b8 = (const unsigned char *) b;

	for (unsigned long int i = 0; i < size; i++) {
		if (a8[i] < b8[i])
			return -1;
		else if (a8[i] > b8[i])
			return 1;
	}

	return 0;
}

unsigned long int pure64_strlen(const char *str) {

	unsigned long int i = 0;

	while (str[i] != 0)
		i++;

	return i;
}

int pure64_strcmp(const char *a, const char *b) {

	while (*a && *a == *b) {
		a++;
		b++;
	}

	if (*a > *b)
		return 1;
	else if (*a < *b)
		return -1;
	else
		return 0;
}
