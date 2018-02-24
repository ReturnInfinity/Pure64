/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_MBR_H
#define PURE64_MBR_H

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
	uint8_t code[490];
	/** The number of sectors occupied
	 * by the second stage boot loader. */
	uint16_t load_sector_count;
	/** The address to load the 2nd
	 * stage boot loader at. */
	uint16_t load_address;
	/** The segment to load the 2nd
	 * stage boot loader at. */
	uint16_t load_segment;
	/** The sector that contains the
	 * 2nd stage boot loader. */
	uint64_t load_sector;
	/** Padding until the boot signature
	 * at offset 510. */
	uint8_t padding[6];
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
