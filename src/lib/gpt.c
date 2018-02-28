/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/gpt.h>

#include <pure64/memory.h>
#include <pure64/stream.h>
#include <pure64/string.h>
#include <pure64/uuid.h>

#include "misc.h"

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

#ifndef GPT_ENTRY_SIZE
#define GPT_ENTRY_SIZE 128
#endif

#ifndef GPT_ENTRY_COUNT
#define GPT_ENTRY_COUNT PURE64_GPT_ENTRY_COUNT
#endif

#ifndef GPT_HEADER_SIZE
#define GPT_HEADER_SIZE 92
#endif

#ifndef GPT_INVALID_LBA
#define GPT_INVALID_LBA PURE64_GPT_INVALID_LBA
#endif

static void init_header(struct pure64_gpt_header *header) {
	header->signature[0] = 'E';
	header->signature[1] = 'F';
	header->signature[2] = 'I';
	header->signature[3] = ' ';
	header->signature[4] = 'P';
	header->signature[5] = 'A';
	header->signature[6] = 'R';
	header->signature[7] = 'T';
	header->version = 0x010000;
	header->checksum = 0;
	header->reserved = 0;
	header->current_lba = GPT_INVALID_LBA;
	header->backup_lba = GPT_INVALID_LBA;
	header->first_usable_lba = GPT_INVALID_LBA;
	header->last_usable_lba = GPT_INVALID_LBA;
	pure64_uuid_zero(&header->disk_uuid);
	header->partition_entries_lba = GPT_INVALID_LBA;
	header->partition_entry_count = 0;
	header->partition_entry_size = GPT_ENTRY_SIZE;
	header->partition_entries_checksum = 0;
}

static uint32_t crc32(const void *buf, uint64_t buf_size) {

	uint64_t i;
	uint64_t j;
	uint32_t byte;
	uint32_t crc;
	uint32_t mask;
	const uint8_t *buf8;

	crc = 0xFFFFFFFF;

	buf8 = (const uint8_t *) buf;

	for (i = 0; i < buf_size; i++) {
		byte = buf8[i];
		crc = crc ^ byte;
		for (j = 0; j < 8; j++) {
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
	}

	return ~crc;
}

static int encode_uint32_at(uint32_t n, struct pure64_stream *stream, uint64_t pos) {

	int err = pure64_stream_set_pos(stream, pos);
	if (err != 0)
		return err;

	err = encode_uint32(n, stream);
	if (err != 0)
		return err;

	return 0;
}

static int calculate_header_checksum(struct pure64_stream *stream, uint64_t header_location) {

	uint32_t checksum;

	/* start with header */

	int err = pure64_stream_set_pos(stream, header_location);
	if (err != 0)
		return err;

	unsigned char buf[GPT_HEADER_SIZE];

	err = pure64_stream_read(stream, buf, sizeof(buf));
	if (err != 0)
		return err;

	checksum = crc32(buf, sizeof(buf));

	err = pure64_stream_set_pos(stream, header_location + 16);
	if (err != 0)
		return err;

	err = encode_uint32(checksum, stream);
	if (err != 0)
		return err;

	return 0;
}

static int calculate_entries_checksum(struct pure64_stream *stream) {

	/* start with header */

	int err = pure64_stream_set_pos(stream, 1024);
	if (err != 0)
		return err;

	uint64_t buf_size = GPT_ENTRY_SIZE * GPT_ENTRY_COUNT;

	void *buf = pure64_malloc(buf_size);

	err = pure64_stream_read(stream, buf, buf_size);
	if (err != 0) {
		pure64_free(buf);
		return err;
	}

	uint32_t checksum = crc32(buf, buf_size);

	/* write the checksum to the primary header */

	err = encode_uint32_at(checksum, stream, 512 + 88);
	if (err != 0) {
		pure64_free(buf);
		return err;
	}

	/* find the backup header LBA from the primary header  */

	err = pure64_stream_set_pos(stream, 512 + 32);
	if (err != 0) {
		pure64_free(buf);
		return err;
	}

	uint64_t backup_lba = 0;

	err = decode_uint64(&backup_lba, stream);
	if (err != 0) {
		pure64_free(buf);
		return err;
	}

	/* go to the backup entry array */

	uint64_t backup_pos = backup_lba * 512;
	backup_pos -= GPT_ENTRY_COUNT * GPT_ENTRY_SIZE;

	err = pure64_stream_set_pos(stream, backup_pos);
	if (err != 0) {
		pure64_free(buf);
		return err;
	}

	err = pure64_stream_read(stream, buf, buf_size);
	if (err != 0) {
		pure64_free(buf);
		return err;
	}

	checksum = crc32(buf, buf_size);

	pure64_free(buf);

	/* encode the checksum for the backup entry array */

	err = pure64_stream_set_pos(stream, (backup_lba * 512) + 88);
	if (err != 0)
		return err;

	err = encode_uint32(checksum, stream);
	if (err != 0)
		return err;

	return 0;
}

static int calculate_checksums(struct pure64_stream *stream) {

	int err = calculate_entries_checksum(stream);
	if (err != 0) {
		return err;
	}

	err = calculate_header_checksum(stream, 512);
	if (err != 0)
		return err;

	err = pure64_stream_set_pos(stream, 512 + 32);
	if (err != 0)
		return err;

	uint64_t backup_lba = 0;

	err = decode_uint64(&backup_lba, stream);
	if (err != 0)
		return err;

	err = calculate_header_checksum(stream, backup_lba * 512);
	if (err != 0)
		return err;

	return 0;
}

int pure64_gpt_header_export(const struct pure64_gpt_header *header,
                             struct pure64_stream *stream) {

	/* signature */
	int err = pure64_stream_write(stream, "EFI PART", 8);
	if (err != 0)
		return err;

	/* version */
	err |= encode_uint32(0x010000, stream);
	if (err != 0)
		return err;

	/* header size */
	err |= encode_uint32(0x5c, stream);
	if (err != 0)
		return err;

	/* crc32 of header (zero during calculation) */
	err |= encode_uint32(0x00, stream);
	if (err != 0)
		return err;

	/* reserved */
	err |= encode_uint32(0x00, stream);
	if (err != 0)
		return err;

	/* current lba */
	err |= encode_uint64(header->current_lba, stream);
	if (err != 0)
		return err;

	/* backup lba */
	err |= encode_uint64(header->backup_lba, stream);
	if (err != 0)
		return err;

	/* first usable lba */
	err |= encode_uint64(header->first_usable_lba, stream);
	if (err != 0)
		return err;

	/* last usable lba */
	err |= encode_uint64(header->last_usable_lba, stream);
	if (err != 0)
		return err;

	/* disk UUID */
	err |= pure64_stream_write(stream, header->disk_uuid.bytes, 16);
	if (err != 0)
		return err;

	/* lba of partition entries */
	err |= encode_uint64(header->partition_entries_lba, stream);
	if (err != 0)
		return err;

	/* number of partition entries */
	err |= encode_uint32(header->partition_entry_count, stream);
	if (err != 0)
		return err;

	/* partition entry size */
	err |= encode_uint32(0x80, stream);
	if (err != 0)
		return err;

	/* crc32 of partition header array */
	err |= encode_uint32(0x00, stream);
	if (err != 0)
		return err;

	/* zero the rest of the sector */
	for (unsigned int i = 0; i < (512 - 92); i += 4) {
		err = pure64_stream_write(stream, "\x00\x00\x00\x00", 4);
		if (err != 0)
			return err;
	}

	return 0;
}

int pure64_gpt_entry_export(const struct pure64_gpt_entry *entry,
                            struct pure64_stream *stream) {

	int err = pure64_stream_write(stream, entry->type_uuid.bytes, sizeof(entry->type_uuid.bytes));
	if (err != 0)
		return err;

	err = pure64_stream_write(stream, entry->entry_uuid.bytes, sizeof(entry->entry_uuid.bytes));
	if (err != 0)
		return err;

	err = encode_uint64(entry->first_lba, stream);
	if (err != 0)
		return err;

	err = encode_uint64(entry->last_lba, stream);
	if (err != 0)
		return err;

	err = encode_uint64(entry->attributes, stream);
	if (err != 0)
		return err;

	err = pure64_stream_write(stream, entry->name, sizeof(entry->name));
	if (err != 0)
		return err;

	return 0;
}

void pure64_gpt_init(struct pure64_gpt *gpt) {
	init_header(&gpt->primary_header);
	init_header(&gpt->backup_header);
	gpt->primary_entries = NULL;
	gpt->backup_entries = NULL;
}

void pure64_gpt_done(struct pure64_gpt *gpt) {
	pure64_free(gpt->primary_entries);
	pure64_free(gpt->backup_entries);
}

void pure64_gpt_set_disk_uuid(struct pure64_gpt *gpt,
                              const struct pure64_uuid *disk_uuid) {

	pure64_memcpy(gpt->primary_header.disk_uuid.bytes,
	              disk_uuid->bytes,
	              sizeof(disk_uuid->bytes));

	pure64_memcpy(gpt->backup_header.disk_uuid.bytes,
	              disk_uuid->bytes,
	              sizeof(disk_uuid->bytes));
}

int pure64_gpt_format(struct pure64_gpt *gpt,
                      struct pure64_stream *stream) {

	uint64_t stream_size = 0;

	int err = pure64_stream_get_size(stream, &stream_size);
	if (err != 0)
		return err;

	/* Initialize primary */

	gpt->primary_header.current_lba = 1;
	gpt->primary_header.backup_lba = (stream_size - 512) / 512;
	gpt->primary_header.first_usable_lba = (1 + 1) * 512; /* mbr + gpt header */
	gpt->primary_header.first_usable_lba += GPT_ENTRY_COUNT * GPT_ENTRY_SIZE;
	gpt->primary_header.first_usable_lba /= 512;
	gpt->primary_header.last_usable_lba = (gpt->primary_header.backup_lba * 512);
	gpt->primary_header.last_usable_lba -= (GPT_ENTRY_COUNT * GPT_ENTRY_SIZE);
	gpt->primary_header.last_usable_lba -= 512;
	gpt->primary_header.last_usable_lba /= 512;
	gpt->primary_header.partition_entries_lba = 2;
	gpt->primary_header.partition_entry_count = GPT_ENTRY_COUNT;

	/* Initialize backup header */

	gpt->backup_header.first_usable_lba = gpt->primary_header.first_usable_lba;
	gpt->backup_header.last_usable_lba = gpt->primary_header.last_usable_lba;
	gpt->backup_header.partition_entry_count = GPT_ENTRY_COUNT;
	gpt->backup_header.current_lba = gpt->primary_header.backup_lba;
	gpt->backup_header.backup_lba = gpt->primary_header.current_lba;
	gpt->backup_header.partition_entries_lba = gpt->backup_header.current_lba * 512;
	gpt->backup_header.partition_entries_lba -= GPT_ENTRY_COUNT * GPT_ENTRY_SIZE;
	gpt->backup_header.partition_entries_lba /= 512;

	/* Initialize primary entries */

	struct pure64_gpt_entry *entries;

	entries = pure64_realloc(gpt->primary_entries, 128 * sizeof(struct pure64_gpt_entry));
	if (entries == NULL) {
		return -1;
	}

	pure64_memset(entries, 0, 128 * sizeof(struct pure64_gpt_entry));

	gpt->primary_entries = entries;

	/* Initialize backup entries */

	entries = pure64_realloc(gpt->backup_entries, 128 * sizeof(struct pure64_gpt_entry));
	if (entries == NULL) {
		return -1;
	}

	pure64_memset(entries, 0, 128 * sizeof(struct pure64_gpt_entry));

	gpt->backup_entries = entries;

	return pure64_gpt_export(gpt, stream);
}

int pure64_gpt_import(struct pure64_gpt *gpt,
                      struct pure64_stream *stream) {
	(void) gpt;
	(void) stream;
	return 0;
}

int pure64_gpt_export(const struct pure64_gpt *gpt,
                      struct pure64_stream *stream) {

	int err = pure64_stream_set_pos(stream, 512);
	if (err != 0)
		return err;

	err = pure64_gpt_header_export(&gpt->primary_header, stream);
	if (err != 0)
		return err;

	for (uint32_t i = 0; i < gpt->primary_header.partition_entry_count; i++) {
		err = pure64_gpt_entry_export(&gpt->primary_entries[i], stream);
		if (err != 0)
			return err;
	}

	err = pure64_stream_set_pos(stream, gpt->backup_header.partition_entries_lba * 512);
	if (err != 0)
		return err;

	for (uint32_t i = 0; i < gpt->backup_header.partition_entry_count; i++) {
		err = pure64_gpt_entry_export(&gpt->backup_entries[i], stream);
		if (err != 0)
			return err;
	}

	err = pure64_gpt_header_export(&gpt->backup_header, stream);
	if (err != 0)
		return err;

	err = calculate_checksums(stream);
	if (err != 0)
		return err;

	return 0;
}
