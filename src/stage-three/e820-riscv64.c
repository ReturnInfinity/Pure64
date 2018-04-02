/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "e820.h"

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

const struct pure64_e820 e820_table[] = {
	{
		(void *) 0x80000000 /* addr */,
		0x10000000 /* size */,
		1 /* type (usable) */,
		0 /* attr (none) */,
		0 /* padding */
	},
	{
		NULL /* addr */,
		0 /* size */,
		0 /* type */,
		0 /* attr */,
		0 /* padding */
	}
};

const struct pure64_e820 *pure64_e820_get(void) {
	return e820_table;
}

const struct pure64_e820 *pure64_e820_next(const struct pure64_e820 *e820) {
	return ++e820;
}

int pure64_e820_usable(const struct pure64_e820 *e820) {
	return e820->type == 1;
}

int pure64_e820_end(const struct pure64_e820 *e820) {
	if ((e820->addr == NULL)
	 && (e820->size == 0))
		return 1;
	else
		return 0;
}
