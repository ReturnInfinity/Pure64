/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_GPT_H
#define PURE64_GPT_H

#include <pure64/uuid.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	/** Padding until the end of the sector. */
	uint8_t padding[420];
};

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_GPT_H */
