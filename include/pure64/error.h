/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file */

#ifndef PURE64_ERROR_H
#define PURE64_ERROR_H

/** A bad address was passed to a function. */

#define PURE64_EFAULT  0x01

/** No more memory available. */

#define PURE64_ENOMEM  0x02

/** The entry is a directory (and it shouldn't be). */

#define PURE64_EISDIR  0x03

/** The entry is not a directory (but it should be). */

#define PURE64_ENOTDIR 0x04

/** No such file or directory. */

#define PURE64_ENOENT  0x05

/** The file or directory already exists. */

#define PURE64_EEXIST  0x06

/** Invalid argument */

#define PURE64_EINVAL  0x07

/** Function not implemented. */

#define PURE64_ENOSYS  0x08

/** Input/Output error occured. */

#define PURE64_EIO 0x09

#ifdef __cplusplus
extern "C" {
#endif

/** Get a string representation of an
 * error that occured in the Pure64 library.
 * @param err The error code that was returned.
 * @returns A human-readable error description.
 * */

const char *pure64_strerror(int err);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_ERROR_H */
