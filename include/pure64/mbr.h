/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file mbr.h API related to the master boot record. */

#ifndef PURE64_MBR_H
#define PURE64_MBR_H

#include <pure64/dap.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_stream;

/** The master boot record.
 * Also contains information
 * used to load the partition
 * containing the 2nd stage
 * boot loader.
 * */

struct pure64_mbr {
	/** The assembly code used
	 * to load the next boot loader. */
	uint8_t code[476];
	/** The disk address packet for
	 * the second stage boot loader. */
	struct pure64_dap st2dap;
	/** The disk address packet for
	 * the third stage boot loader. */
	struct pure64_dap st3dap;
	/** The boot signature. */
	uint16_t boot_signature;
};

/** Zero out the MBR structure.
 * @param mbr An MBR structure.
 * */

void pure64_mbr_zero(struct pure64_mbr *mbr);

/** Check if the MBR is valid.
 * This function checks if the MBR
 * has a boot signature.
 * @param mbr The MBR structure to check.
 * @returns Zero if the MBR is valid.
 * If it is not, then @ref PURE64_EINVAL
 * is returned.
 * */

int pure64_mbr_check(const struct pure64_mbr *mbr);

/** Read an MBR sector from a stream.
 * The MBR is read from the beginning of the stream.
 * @param mbr The MBR structure to put the data into.
 * @param stream The stream to read the MBR from.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_mbr_read(struct pure64_mbr *mbr, struct pure64_stream *stream);

/** Write an MBR sector to a stream.
 * The MBR is written to the beginning of the stream.
 * @param mbr The MBR structure to get the data from.
 * @param stream The stream to write the MBR to.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_mbr_write(const struct pure64_mbr *mbr, struct pure64_stream *stream);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_MBR_H */
