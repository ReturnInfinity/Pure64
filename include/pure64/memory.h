/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_MEMORY_H
#define PURE64_MEMORY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *pure64_malloc(uint64_t size);

void *pure64_realloc(void *addr, uint64_t size);

void pure64_free(void *addr);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_MEMORY_H */
