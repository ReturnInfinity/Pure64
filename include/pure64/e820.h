/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file */

#ifndef PURE64_E820_H
#define PURE64_E820_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** An entry in the E820 memory map.
 * */

struct pure64_e820 {
	/** The address of this record. */
	void *addr;
	/** The number of bytes occupied by
	 * this record. */
	uint64_t size;
	/** Type 1 is usable, type 2 is not. */
	uint32_t type;
	/** ACPI 3.0 Extended Attributes */
	uint64_t attr;
	/** Padding for 32-byte alignment */
	uint64_t padding;
};

/** Get the next entry in the E820 map.
 * When the end of the map is reached,
 * a null pointer is returned.
 * @param index The index of the record to get.
 * Zero is the first valid number to pass to this function.
 * @returns A pointer to the record, if it is found.
 * Otherwise, it is zero.
 * */

const struct pure64_e820 *pure64_e820_probe(uint64_t index);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_E820_H */
