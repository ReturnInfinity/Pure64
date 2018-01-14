/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_MISC_H
#define PURE64_MISC_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int encode_uint64(uint64_t n, FILE *file);

int decode_uint64(uint64_t *n_ptr, FILE *file);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_MISC_H */
