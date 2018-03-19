/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file uuid.h UUID related functions and structures. */

#ifndef PURE64_UUID_H
#define PURE64_UUID_H

#ifndef PURE64_UUID_DISK

/** The UUID of the disk formatted
 * with a GUID partition table.
 * */

#define PURE64_UUID_DISK "dc301a8c-1632-45a6-962c-aa4f1cc35943"

#endif

#ifndef PURE64_UUID_STAGE_TWO

/** The UUID for the stage
 * two boot loader partition.
 * */

#define PURE64_UUID_STAGE_TWO "daa1ab4e-7a2c-4404-8208-61a12c660382"

#endif

#ifndef PURE64_UUID_STAGE_THREE

/** The UUID of the stage
 * three partition.
 * */

#define PURE64_UUID_STAGE_THREE "32cfd7f2-0e0a-4908-8d3b-16d7fb3a3c57"

#endif

#ifndef PURE64_UUID_FILE_SYSTEM

/** The UUID for Pure64's file system.
 * */

#define PURE64_UUID_FILE_SYSTEM "f7439905-43da-4df0-b863-1f456e008b58"

#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Universally unique identifer.
 * Used to identify various parts
 * of the Pure64 partition table.
 * */

struct pure64_uuid {
	/** The byte oriented layout
	 * of the identifier. */
	uint8_t bytes[16];
};

/** Zero the value of an identifier.
 * This may be desired for initialization
 * purposes. A UUID of all zeros indicates
 * that it was not initialized properly.
 * @param uuid A UUID structure.
 * */

void pure64_uuid_zero(struct pure64_uuid *uuid);

/** Copies a UUID.
 * @param dst The destination of the UUID.
 * @param src The UUID to copy over to the
 * destination.
 * */

void pure64_uuid_copy(struct pure64_uuid *dst,
                      const struct pure64_uuid *src);

/** Compares two UUIDs.
 * @param a The first UUID
 * @param b The second UUID
 * @returns Zero, if the UUIDs are equal.
 * One, if @p a is greater than @p b.
 * Negative one, if @p is less than @p b.
 * */

int pure64_uuid_cmp(const struct pure64_uuid *a,
                    const struct pure64_uuid *b);

/** Parse a UUID string.
 * @param uuid A UUID structure.
 * @param uuid_str A UUID string.
 * The first three numbers should be
 * little endian and the last two should
 * be big endian. It does not matter if
 * the hex characters are upper or lower case.
 * @returns Zero on success, @ref PURE64_EINVAL
 * if the string was formatted incorrectly, and
 * @ref PURE64_EFAULT if an address was NULL.
 * */

int pure64_uuid_parse(struct pure64_uuid *uuid, const char *uuid_str);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_UUID_H */
