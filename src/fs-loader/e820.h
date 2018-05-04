/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_E820_H
#define PURE64_E820_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A structure describing
 * an area of memory within
 * the system.
 * */

struct pure64_e820 {
	/** The address of the memory
	 * section. */
	void *addr;
	/** The number of bytes occupied
	 * by this section. */
	uint64_t size;
	/** The type associated with this
	 * section. The value of one means
	 * that this is usable memory. The
	 * type of two means that it is not.
	 * */
	uint32_t type;
	/** Attributes associated with this
	 * section of memory.
	 * */
	uint32_t attr;
	/** Padding for 32-byte alignment. */
	uint64_t padding;
};

/** Gets a pointer to the E820 table.
 * This should be the first function that
 * is called if the caller wants to determine
 * the memory map. @ref pure64_e820_next can
 * be called to get the next entry.
 * @returns A pointer to the E820 table.
 * */

const struct pure64_e820 *pure64_e820_get(void);

/** Returns next entry in the E820 array.
 * @param e820 A valid E820 structure.
 * Use @ref pure64_e820_end to make sure
 * that the entry your passing is not the
 * end of the list.
 * @returns The next E820 entry.
 * */

const struct pure64_e820 *pure64_e820_next(const struct pure64_e820 *e820);

/** Indicates whether or not an E820 entry
 * is the terminating entry.
 * @param e820 A valid E820 entry.
 * @returns One if the entry is the terminating
 * entry, zero if it is not.
 * */

int pure64_e820_end(const struct pure64_e820 *e820);

/** Indicates whether ot not an E820 entry usable.
 * @param e820 A valid E820 entry. The caller
 * should first check, with @ref pure64_e820_end,
 * if the entry is a terminator or not.
 * @returns One of the entry is usable, zero if
 * it is not.
 * */

int pure64_e820_usable(const struct pure64_e820 *e820);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_E820_H */
