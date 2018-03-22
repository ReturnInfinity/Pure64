/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_TYPES_H
#define PURE64_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef pure64_true
#define pure64_true 1
#endif

#ifndef pure64_false
#define pure64_false 0
#endif

#ifndef pure64_null
#define pure64_null ((void *) 0x00)
#endif

#ifndef NULL
#define NULL pure64_null
#endif

typedef unsigned long long int pure64_uint64;

typedef unsigned int pure64_uint32;

typedef unsigned short int pure64_uint16;

typedef unsigned char pure64_uint8;

typedef unsigned char pure64_bool;

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_TYPES_H */
