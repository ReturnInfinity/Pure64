/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_ALLOC_H
#define PURE64_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/** A memory allocation.
 * */

struct pure64_alloc {
	/** The base address of the memory block*/
	void *addr;
	/** The number of bytes being used by the
	 * memory block. */
	uint64_t size;
	/** The number of bytes reserved for the
	 * memory block to grow. */
	uint64_t reserved;
};

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_ALLOC_H */
