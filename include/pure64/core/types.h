/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file types.h
 * Common types and values used throughout the library.
 * */

#ifndef PURE64_TYPES_H
#define PURE64_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/** A boolean true value.
 * @ingroup core-api
 * */

#define pure64_true 1

/** A boolean false  value.
 * @ingroup core-api
 * */

#define pure64_false 0

/** An invalid pointer value.
 * @ingroup core-api
 * */

#define pure64_null ((void *) 0x00)

/** The largest type available in the system.
 * @ingroup core-api
 * */

typedef unsigned long long int pure64_size;

/** A 64-bit unsigned integer.
 * @ingroup core-api
 * */

typedef unsigned long long int pure64_uint64;

/** A 32-bit unsigned integer.
 * @ingroup core-api
 * */

typedef unsigned int pure64_uint32;

/** A 16-bit unsigned integer.
 * @ingroup core-api
 * */

typedef unsigned short int pure64_uint16;

/** An 8-bit unsigned integer.
 * @ingroup core-api
 * */

typedef unsigned char pure64_uint8;

/** A boolean type.
 * Use @ref pure64_false and @ref pure64_true
 * for setting this value.
 * @ingroup core-api
 * */

typedef unsigned char pure64_bool;

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_TYPES_H */
