/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_MISC_H
#define PURE64_MISC_H

#include <pure64/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_stream;

int encode_uint16(pure64_uint16 n, struct pure64_stream *file);

int encode_uint32(pure64_uint32 n, struct pure64_stream *file);

int encode_uint64(pure64_uint64 n, struct pure64_stream *file);

int decode_uint16(pure64_uint16 *n_ptr, struct pure64_stream *file);

int decode_uint32(pure64_uint32 *n_ptr, struct pure64_stream *file);

int decode_uint64(pure64_uint64 *n_ptr, struct pure64_stream *file);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_MISC_H */
