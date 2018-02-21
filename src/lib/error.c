/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/error.h>

const char *pure64_strerror(int err) {

	switch (err) {
	case 0:
		/* Zero indicates no error */
		break;
	case PURE64_EFAULT:
		return "Bad address was detected.";
	case PURE64_ENOMEM:
		return "Not enough memory available.";
	case PURE64_EISDIR:
		return "Entry is a directory.";
	case PURE64_ENOTDIR:
		return "Entry is not a directory.";
	case PURE64_EEXIST:
		return "Entry exists already.";
	case PURE64_EINVAL:
		return "Invalid argument was detected.";
	case PURE64_ENOSYS:
		return "Functionality not implemented.";
	case PURE64_EIO:
		return "I/O error occured";
	default:
		return "Unknown error has occurred.";
	}

	return "Success";
}

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

const char *pure64_strerror(int err);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_ERROR_H */
