/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file gpt.h API related to GPT functions. */

#ifndef PURE64_GPT_H
#define PURE64_GPT_H

#include <pure64/uuid.h>

#include <stdint.h>
#include <uchar.h>

#ifndef PURE64_GPT_INVALID_LBA
#define PURE64_GPT_INVALID_LBA 0
#endif

#ifndef PURE64_GPT_ENTRY_COUNT
#define PURE64_GPT_ENTRY_COUNT 128
#endif

#ifndef PURE64_GPT_MINIMUM_SIZE
#define PURE64_GPT_MINIMUM_SIZE ((512 + (128 * PURE64_GPT_ENTRY_COUNT)) * 2)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_stream;

/** A GUID partition table header.
 * */

struct pure64_gpt_header {
	/** The GPT signature, indicating
	 * that it is a GPT partitioned disk. */
	char signature[8];
	/** The version of the GPT layout. */
	uint32_t version;
	/** The 32-bit CRC checksum of this header. */
	uint32_t checksum;
	/** Reserved */
	uint32_t reserved;
	/** The LBA that contains this header. */
	uint64_t current_lba;
	/** The LBA that contains the backup header. */
	uint64_t backup_lba;
	/** The first LBA that may be used by a partition. */
	uint64_t first_usable_lba;
	/** The last LBA that may be used by a partition. */
	uint64_t last_usable_lba;
	/** The UUID of the disk containing this partition table. */
	struct pure64_uuid disk_uuid;
	/** The starting LBA of the partition entries. */
	uint64_t partition_entries_lba;
	/** The number of partition entries. This does not
	 * indicate the number of partition entries that are
	 * currently used, only how many are reserved for use. */
	uint32_t partition_entry_count;
	/** The size of a single partition entry. */
	uint32_t partition_entry_size;
	/** The 32-bit CRC checksum of the partition entries. */
	uint32_t partition_entries_checksum;
};

/** Exports a GPT header to a stream.
 * This function should only be used
 * within the library.
 * @param header The header to export
 * @param stream The stream to export
 * the header to.
 * @returns Zero on success, an error
 * code on failure.
 * */

int pure64_gpt_header_export(const struct pure64_gpt_header *header, struct pure64_stream *stream);

/** This structure represents a
 * partition entry in a GPT formatted
 * medium.
 * */

struct pure64_gpt_entry {
	/** The partition type UUID. */
	struct pure64_uuid type_uuid;
	/** This entry's UUID */
	struct pure64_uuid entry_uuid;
	/** The first LBA containing
	 * the partition data. */
	uint64_t first_lba;
	/** The last LBA containing
	 * partition data. */
	uint64_t last_lba;
	/** Partition attributes */
	uint64_t attributes;
	/** The name of the partition,
	 * encoded as UTF-16LE. */
	char16_t name[36];
};

/** Export a GPT entry to a stream.
 * This function should only be used
 * within the library.
 * @param entry The entry to export.
 * @param stream The stream to export
 * the entry to.
 * @returns Zero on success, an error
 * code on failure.
 * */

int pure64_gpt_entry_export(const struct pure64_gpt_entry *entry, struct pure64_stream *stream);

/** This structure represents a
 * GUID partition table.
 * */

struct pure64_gpt {
	/** The primary GPT header. */
	struct pure64_gpt_header primary_header;
	/** The backup GPT header. */
	struct pure64_gpt_header backup_header;
	/** The primary array of partition entries. */
	struct pure64_gpt_entry *primary_entries;
	/** The backup array of partition entries. */
	struct pure64_gpt_entry *backup_entries;
};

/** Initializes a GPT  structure.
 * @param gpt The GPT structure to initialize.
 * @returns Zero on success, an error code
 * on failure.
 * */

void pure64_gpt_init(struct pure64_gpt *gpt);

/** Releases resources allocated by a
 * GPT gpt structure.
 * @param gpt The GPT to release the
 * resources for.
 * */

void pure64_gpt_done(struct pure64_gpt *gpt);

/** Sets the disk UUID for the GPT structure.
 * @param gpt An initialized GPT structure.
 * @param uuid The disk UUID to use for the
 * GPT structure.
 * */

void pure64_gpt_set_disk_uuid(struct pure64_gpt *gpt,
                              const struct pure64_uuid *uuid);

/** This function formats a stream
 * to GPT. If there is an existing
 * partition table, then it is overwritten
 * with a new one.
 * @param gpt An initialized GPT structure.
 * @param stream The stream to format with GPT.
 * @returns Zero on success, an error code on
 * failure.
 * */

int pure64_gpt_format(struct pure64_gpt *gpt,
                      struct pure64_stream *stream);

/** Import GPT data from a stream.
 * @param gpt The GPT structure to hold the data.
 * @param stream The stream to read the GPT data from.
 * @returns Zero on success, an error
 * code on failure.
 * */

int pure64_gpt_import(struct pure64_gpt *gpt,
                      struct pure64_stream *stream);

/** Export GPT data to a stream.
 * @param gpt The GPT structure containing
 * the data to export to the stream.
 * @param stream The stream to export the
 * GPT data to.
 * @returns Zero on success, a negative error
 * code on failure.
 * */

int pure64_gpt_export(const struct pure64_gpt *gpt,
                      struct pure64_stream *stream);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_GPT_H */
