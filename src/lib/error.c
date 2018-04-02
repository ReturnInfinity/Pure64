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
	case PURE64_ENOENT:
		return "No such file or directory.";
	case PURE64_EEXIST:
		return "Entry exists already.";
	case PURE64_EINVAL:
		return "Invalid argument was detected.";
	case PURE64_ENOSYS:
		return "Functionality not implemented.";
	case PURE64_EIO:
		return "I/O error occured";
	}

	return "Success";
}
