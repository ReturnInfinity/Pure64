/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/memory.h>

#include <stdlib.h>

void *pure64_malloc(uint64_t size) {
	return malloc(size);
}

void *pure64_realloc(void *addr, uint64_t size) {
	return realloc(addr, size);
}

void pure64_free(void *addr) {
	free(addr);
}
