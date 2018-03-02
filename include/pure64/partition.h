/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file partition.h API related to partion
 * data structures and functions.
 * */

#ifndef PURE64_PARTITION_H
#define PURE64_PARTITION_H

#include <pure64/stream.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** This structure represents a
 * partition contained within a
 * disk recognized by Pure64.
 * */

struct pure64_partition {
	/** The stream of the partition. */
	struct pure64_stream stream;
	/** The stream of the disk that
	 * contains the partition. */
	struct pure64_stream *disk;
	/** The beginning of the partition,
	 * in bytes.  */
	uint64_t offset;
	/** The number of bytes that
	 * are occupied by the partition. */
	uint64_t size;
};

/** Initializes a Pure64 partition.
 * @param partition The partition structure
 * to initialize.
 * */

void pure64_partition_init(struct pure64_partition *partition);

/** Release memory allocated by the
 * partition structure.
 * @param partition The partition to
 * release the memory for.
 * */

void pure64_partition_done(struct pure64_partition *partition);

/** Assign the stream to use for the
 * partition. The start and offset of
 * the partition should also be set,
 * so that the read and write functions
 * function within the bounds of the
 * partition.
 * @param partition The partition structure
 * to assign the stream to.
 * @param stream The stream to assign to
 * the partition. This would typically
 * be a disk but it does not have to be.
 * */

void pure64_partition_set_stream(struct pure64_partition *partition,
                                 struct pure64_stream *stream);

/** Set the number of bytes contained by the partition.
 * @param partition The partition to set the size of.
 * @param size The size, in bytes, of the partition.
 * This can be any value from zero to the size of the
 * medium containing the partition.
 * */

void pure64_partition_set_size(struct pure64_partition *partition,
                                uint64_t size);

/** Set the starting point of the partition, in terms
 * of bytes. After using this function, read and write
 * operations starting at zero will actually begin at
 * this location.
 * @param partition The partition to set the offset for.
 * @param offset The offset of the partition. This may
 * be any value from zero to the size of the partition.
 * */

void pure64_partition_set_offset(struct pure64_partition *partition,
                                uint64_t offset);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_PARTITION_H */
