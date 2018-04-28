/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file memory.h API related to memory allocation functions. */

#ifndef PURE64_MEMORY_H
#define PURE64_MEMORY_H

#include <pure64/core/types.h>

/** Represents an invalid memory
 * reference.
 * */

#ifdef __cplusplus
extern "C" {
#endif

/** Allocates memory of a specified size.
 * @param size The number of bytes to allocate.
 * @returns The addresss of the memory on success,
 * zero if there is no more memory available.
 * */

void *pure64_malloc(pure64_uint64 size);

/** Resizes an existing memory block. If the
 * address passed to this function is zero, then
 * a new memory block is allocated.
 * @param addr The existing memory address, if applicable.
 * @param size The new size that the existing memory block
 * should fit. If a new memory block is being allocated,
 * then this should be the initial size of the memory block.
 * @returns The address of the new memory block, if successful.
 * Otherwise, zero is returned.
 * */

void *pure64_realloc(void *addr, pure64_uint64 size);

/** Releases a memory block previously allocated.
 * @param addr The address of the memory block that
 * was allocated with @ref pure64_malloc or @ref pure64_realloc.
 * This may also be zero, in which case this function does nothing.
 * */

void pure64_free(void *addr);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_MEMORY_H */
