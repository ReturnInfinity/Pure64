/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file gpt.h API related to GPT functions. */

#ifndef PURE64_GPT_H
#define PURE64_GPT_H

#include <pure64/uuid.h>

#ifndef PURE64_GPT_INVALID_LBA

/** This value represents an invalid
 * or uninitialized LBA value. It is
 * set to a value that cannot possibly
 * occur on a 64, 32, or 16-bit system.
 * */

#define PURE64_GPT_INVALID_LBA 0

#endif

#ifndef PURE64_GPT_ENTRY_COUNT

/** This value represents the number
 * of available partition entries that
 * Pure64 allows for when formatting a
 * disk. This value is not the number
 * of entries that Pure64 supports on
 * a GPT formatted disk, just the number
 * of partition entries that Pure64 gives
 * by default.
 * */

#define PURE64_GPT_ENTRY_COUNT 128

#endif

#ifndef PURE64_GPT_MINIMUM_SIZE

/** This value is the size, in bytes,
 * that GPT data occupies on the disk.
 * Therefore, a disk must be at least
 * this size if it should contain a GPT
 * formatted partition table.
 * */

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
	/** The size of this header on disk. */
	uint32_t header_size;
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
	uint16_t name[36];
};

/** Initializes a GPT partition entry.
 * The function initializes the entry
 * in a way that it can be distinguished
 * as an empty one.
 * This function should only be used
 * internally.
 * @param entry The entry to initialize.
 * */

void pure64_gpt_entry_init(struct pure64_gpt_entry *entry);

/** Indicates whether or not a partition
 * is used. This is useful for iterating
 * the partition entries, since some of them might
 * not actually point to a partition.
 * @param entry An initialized GPT entry.
 * @returns One if the entry is used, zero
 * of it is not.
 * */

int pure64_gpt_entry_is_used(const struct pure64_gpt_entry *entry);

int pure64_gpt_entry_is_type(const struct pure64_gpt_entry *entry,
                             const char *type_uuid);

/** Get the offset, in terms of bytes, of the
 * start of the partition.
 * @param entry The entry to get the offset of.
 * @returns The offset of the partition data on disk.
 * */

uint64_t pure64_gpt_entry_get_offset(const struct pure64_gpt_entry *entry);

/** Get the size of the partition, in terms of bytes.
 * @param entry The entry to get the size of.
 * @returns The size of the partition, in terms of bytes.
 * */

uint64_t pure64_gpt_entry_get_size(const struct pure64_gpt_entry *entry);

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

/** Allocates a partition of a specified size and UUID.
 * @param gpt An initialized GPT structure.
 * @param uuid The UUID to use for the new GPT entry.
 * @param lba_count The number of LBAs that
 * the partition should be able to fit.
 * @returns Zero on success, an error code
 * on failure.
 * */

uint32_t pure64_gpt_alloc(struct pure64_gpt *gpt,
                          const struct pure64_uuid *uuid,
                          uint64_t lba_count);

/** This function formats the GPT structure
 * according to the size of the disk.
 * @param gpt An initialized GPT structure.
 * @param disk_size The size, in bytes, of
 * the disk that the GPT structure will be
 * exported to.
 * @returns Zero on success, an error code on
 * failure.
 * */

int pure64_gpt_format(struct pure64_gpt *gpt,
                      uint64_t disk_size);

/** Locates the first unused entry in the partition
 * header entry array.
 * @param gpt An initialized GPT structure.
 * @param entry_index A pointer to a variable that
 * will receive the index of the unused entry, if
 * it is found.
 * @returns Zero if an unused entry is found. An
 * error code is returned if the function was unable
 * to find an unused entry.
 * */

int pure64_gpt_find_unused_entry(const struct pure64_gpt *gpt,
                                 uint32_t *entry_index);

/** Gets the offset, in bytes, of a partition described
 * in the partition table.
 * @param gpt An initialized GPT structure.
 * @param entry_index The index of the partition header entry.
 * @param offset A pointer to the variable that will receive
 * the partition offset.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_gpt_get_partition_offset(const struct pure64_gpt *gpt,
                                    uint32_t entry_index,
                                    uint64_t *offset);

/** Gets the size, in bytes, of a partition described
 * in the partition table.
 * @param gpt An initialized GPT structure.
 * @param entry_index The index of the partition header entry.
 * @param size A pointer to the variable that will receive
 * the partition size.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_gpt_get_partition_size(const struct pure64_gpt *gpt,
                                  uint32_t entry_index,
                                  uint64_t *size);

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

/** Gets an entry structure by reference
 * of its index within the partition table.
 * @param gpt An initialized GPT structure.
 * @param index The index of the partition entry.
 * @returns A pointer to the entry is returned
 * on success. A null pointer is returned on
 * failure.
 * */

const struct pure64_gpt_entry *pure64_gpt_get_entry(const struct pure64_gpt *gpt,
                                                    uint32_t index);

int pure64_gpt_set_entry_type(struct pure64_gpt *gpt,
                              uint32_t entry_index,
                              const char *type_uuid);

int pure64_gpt_set_entry_name(struct pure64_gpt *gpt,
                              uint32_t entry_index,
                              const uint16_t *name);

/** Sets the size of a certain partition entry.
 * This function will find the appropriate space
 * that can fit the partition data.
 * @param gpt An initialized GPT structure.
 * @param entry_index The index of the GPT entry.
 * @param size The size, in bytes, that the partition
 * should be able to fit.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_gpt_set_entry_size(struct pure64_gpt *gpt,
                              uint32_t entry_index,
                              uint64_t size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_GPT_H */
