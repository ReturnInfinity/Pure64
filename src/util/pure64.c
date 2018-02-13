/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/fs.h>
#include <pure64/file.h>
#include <pure64/error.h>
#include <pure64/stream.h>

#include "mbr-data.h"
#include "pure64-data.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* * * * * *
 * Constants
 * * * * * */

#ifndef PURE64_DEFAULT_DISK_UUID
#define PURE64_DEFAULT_DISK_UUID "74a7c14a-711d-4293-a731-569ca656799e"
#endif

#ifndef GPT_PARTITION_HEADER_SIZE
#define GPT_PARTITION_HEADER_SIZE 128
#endif

#ifndef GPT_PARTITION_HEADER_COUNT
#define GPT_PARTITION_HEADER_COUNT 128
#endif

#ifndef GPT_HEADER_SIZE
#define GPT_HEADER_SIZE 92
#endif

/* * * * * * * * * * * * * * *
 * Uncategorized Declarations
 * * * * * * * * * * * * * * */

static void zero_file_data(FILE *file, unsigned long int size) {
	while (size-- > 0) {
		fputc(0, file);
	}
}

/* * * * * * * * * * * *
 * Checksum Declarations
 * * * * * * * * * * * */

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

static int calculate_header_checksum(FILE *file, long int header_location) {

	char *buf;
	size_t buf_size;
	size_t read_size;
	uint32_t checksum;

	/* start with header */

	if (fseek(file, header_location, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to GPT header.\n");
		return EXIT_FAILURE;
	}

	buf_size = GPT_HEADER_SIZE;

	buf = malloc(buf_size);

	read_size = fread(buf, 1, buf_size, file);
	if (read_size != buf_size) {
		fprintf(stderr, "Failed to read GPT header.\n");
		return EXIT_FAILURE;
	}

	checksum = crc32(buf, buf_size);

	if (fseek(file, header_location + 16, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to header checksum.\n");
		return EXIT_FAILURE;
	}

	if (fwrite(&checksum, 1, 4, file) != 4) {
		fprintf(stderr, "Failed to write header checksum.\n");
		return EXIT_FAILURE;
	}

	free(buf);

	return EXIT_SUCCESS;
}

static int calculate_partition_headers_checksum(FILE *file) {

	char *buf;
	size_t buf_size;
	size_t read_size;
	uint32_t checksum;
	uint64_t backup_lba;

	/* start with header */

	if (fseek(file, 1024, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to GPT partition headers.\n");
		return EXIT_FAILURE;
	}

	buf_size = GPT_PARTITION_HEADER_SIZE * GPT_PARTITION_HEADER_COUNT;

	buf = malloc(buf_size);

	read_size = fread(buf, 1, buf_size, file);
	if (read_size != buf_size) {
		fprintf(stderr, "Failed to read primary GPT partition headers.\n");
		return EXIT_FAILURE;
	}

	checksum = crc32(buf, buf_size);

	if (fseek(file, 512 + 88, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to header checksum.\n");
		return EXIT_FAILURE;
	}

	if (fwrite(&checksum, 1, 4, file) != 4) {
		fprintf(stderr, "Failed to write header checksum.\n");
		return EXIT_FAILURE;
	}

	if (fseek(file, 512 + 32, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to header backup location.\n");
		return EXIT_FAILURE;
	}

	if (fread(&backup_lba, 1, 8, file) != 8) {
		fprintf(stderr, "Failed to read header backup location.\n");
		return EXIT_FAILURE;
	}

	if (fseek(file, (backup_lba * 512) - (GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE), SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to backup partition headers.\n");
		return EXIT_FAILURE;
	}

	if (fread(buf, 1, buf_size, file) != buf_size) {
		fprintf(stderr, "Failed to read backup partition headers.\n");
		return EXIT_FAILURE;
	}

	checksum = crc32(buf, buf_size);

	if (fseek(file, (backup_lba * 512) + 88, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to backup header checksum.\n");
		return EXIT_FAILURE;
	}

	if (fwrite(&checksum, 1, 4, file) != 4) {
		fprintf(stderr, "Failed to write backup header checksum.\n");
		return EXIT_FAILURE;
	}

	free(buf);

	return EXIT_SUCCESS;
}

static int calculate_checksums(FILE *file) {

	int err;
	uint64_t backup_lba;

	err = calculate_partition_headers_checksum(file);
	if (err != 0) {
		return err;
	}

	err = calculate_header_checksum(file, 512);
	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to calculate checksum of primary GPT header.\n");
		return err;
	}

	if ((fseek(file, 512 + 32, SEEK_SET) != 0)
	 || (fread(&backup_lba, 1, 8, file) != 8)) {
		fprintf(stderr, "Failed to read backup LBA from primary GPT header.\n");
		return EXIT_FAILURE;
	}

	err = calculate_header_checksum(file, backup_lba * 512);
	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to calculate checksum of backup GPT header.\n");
		return err;
	}

	return EXIT_SUCCESS;
}

/* * * * * * * * * *
 * UUID Declarations
 * * * * * * * * * */

struct pure64_uuid {
	uint8_t bytes[16];
};

static int pure64_uuid_parse(struct pure64_uuid *uuid, const char *str) {

	unsigned int i;
	unsigned int j;

	i = 0;
	j = 0;

	while ((i < 16) && (str[j] != 0)) {

		uuid->bytes[i] = 0;

		char c = tolower(str[j]);
		if ((c >= '0') && (c <= '9'))
			uuid->bytes[i] |= ((c - '0') + 0x00) << 4;
		else if ((c >= 'a') && (c <= 'f'))
			uuid->bytes[i] |= ((c - 'a') + 0x0a) << 4;
		else if (c == '-') {
			j++;
			continue;
		} else
			return -1;

		c = tolower(str[j + 1]);
		if ((c >= '0') && (c <= '9'))
			uuid->bytes[i] |= (c - '0') + 0x00;
		else if ((c >= 'a') && (c <= 'f'))
			uuid->bytes[i] |= (c - 'a') + 0x0a;
		else
			return -1;

		j += 2;
		i++;
	}

	/* adjust for little endian encoding */

	unsigned char buf[16];

	/* little endian */

	buf[0] = uuid->bytes[3];
	buf[1] = uuid->bytes[2];
	buf[2] = uuid->bytes[1];
	buf[3] = uuid->bytes[0];

	/* little endian */

	buf[4] = uuid->bytes[5];
	buf[5] = uuid->bytes[4];

	/* little endian */

	buf[6] = uuid->bytes[7];
	buf[7] = uuid->bytes[6];

	/* big endian */

	buf[8] = uuid->bytes[8];
	buf[9] = uuid->bytes[9];

	/* big endian */

	buf[10] = uuid->bytes[10];
	buf[11] = uuid->bytes[11];
	buf[12] = uuid->bytes[12];
	buf[13] = uuid->bytes[13];
	buf[14] = uuid->bytes[14];
	buf[15] = uuid->bytes[15];

	for (i = 0; i < 16; i++)
		uuid->bytes[i] = buf[i];
	return 0;
}

/* * * * * * * * * * * * *
 * GPT Header Definitions
 * * * * * * * * * * * * */

/* a short version of the GPT header. */

struct gpt_header {
	uint64_t current_lba;
	uint64_t backup_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	struct pure64_uuid disk_uuid;
	uint64_t partition_headers_lba;
	uint32_t partition_header_count;
};

int export_gpt_header(FILE *file, const struct gpt_header *header) {

	unsigned int i;
	size_t write_count;

	write_count = 0;

	/* signature */
	write_count += fwrite("EFI PART", 1, 8, file);
	/* version */
	write_count += fwrite("\x00\x00\x01\x00", 1, 4, file);
	/* header size */
	write_count += fwrite("\x5c\x00\x00\x00", 1, 4, file);
	/* crc32 of header (zero during calculation) */
	write_count += fwrite("\x00\x00\x00\x00", 1, 4, file);
	/* reserved */
	write_count += fwrite("\x00\x00\x00\x00", 1, 4, file);
	/* current lba */
	write_count += fwrite(&header->current_lba, 1, 8, file);
	/* backup lba */
	write_count += fwrite(&header->backup_lba, 1, 8, file);
	/* first usable lba */
	write_count += fwrite(&header->first_usable_lba, 1, 8, file);
	/* last usable lba */
	write_count += fwrite(&header->last_usable_lba, 1, 8, file);
	/* disk UUID */
	write_count += fwrite(&header->disk_uuid.bytes, 1, 16, file);
	/* lba of partition entries */
	write_count += fwrite(&header->partition_headers_lba, 1, 8, file);
	/* number of partition entries */
	write_count += fwrite(&header->partition_header_count, 1, 4, file);
	/* partition entry size */
	write_count += fwrite("\x80\x00\x00\x00", 1, 4, file);
	/* crc32 of partition header array */
	write_count += fwrite("\x00\x00\x00\x00", 1, 4, file);
	/* zero the rest of the partition */
	for (i = 0; i < (512 - 92); i += 4)
		write_count += fwrite("\x00\x00\x00\x00", 1, 4, file);

	if (write_count != 512) {
		fprintf(stderr, "Impartial write detected while exporting GPT header.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

/* * * * * * * * * * * * * *
 * File Stream Declarations
 * * * * * * * * * * * * * */

static int fstream_set_pos(void *file_ptr, uint64_t pos_ptr) {

	if (pos_ptr > LONG_MAX)
		return PURE64_EINVAL;

	if (fseek((FILE *) file_ptr, pos_ptr, SEEK_SET) != 0)
		return PURE64_EIO;

	return 0;
}

static int fstream_get_pos(void *file_ptr, uint64_t *pos_ptr) {

	long int pos;

	pos = ftell((FILE *) file_ptr);
	if (pos == -1L)
		return PURE64_EIO;

	*pos_ptr = pos;

	return 0;
}

static int fstream_write(void *file_ptr, const void *buf, uint64_t buf_size) {
	if (fwrite(buf, 1, buf_size, (FILE *) file_ptr) != buf_size)
		return PURE64_EIO;
	else
		return 0;
}

static int fstream_read(void *file_ptr, void *buf, uint64_t buf_size) {
	if (fread(buf, 1, buf_size, (FILE *) file_ptr) != buf_size)
		return PURE64_EIO;
	else
		return 0;
}

/* * * * * * * * * * * * * * * * *
 * Memory Allocation Declarations
 * * * * * * * * * * * * * * * * */

void *pure64_malloc(uint64_t size) {
	return malloc(size);
}

void *pure64_realloc(void *addr, uint64_t size) {
	return realloc(addr, size);
}

void pure64_free(void *addr) {
	free(addr);
}

/* * * * * * * * * * * * * *
 * Command Line Declarations
 * * * * * * * * * * * * * */

/** Compares an option with a command
 * line argument. This function checks
 * both short and long options.
 * @param arg The argument from the command line.
 * @param opt The long option (the two '--' are added
 * automatically).
 * @param s_opt The letter representing the short option.
 * @returns True if the argument is a match, false
 * if the argument is not a match.
 * */

static bool check_opt(const char *arg,
                      const char *opt,
                      char s_opt) {

	/* create a string version
	 * of the short option */
	char s_opt3[3];
	s_opt3[0] = '-';
	s_opt3[1] = s_opt;
	s_opt3[2] = 0;
	if (strcmp(s_opt3, arg) == 0)
		return true;

	if ((arg[0] == '-')
	 && (arg[1] == '-')
	 && (strcmp(&arg[2], opt) == 0))
		return true;

	return false;
}

/** Prints the help message.
 * @param argv0 The name of the program.
 * */

static void print_help(const char *argv0) {
	printf("Usage: %s [options] <command>\n", argv0);
	printf("\n");
	printf("Options:\n");
	printf("\t--file, -f : Specify the path to the Pure64 file.\n");
	printf("\t--help, -h : Print this help message.\n");
	printf("\n");
	printf("Commands:\n");
	printf("\tcat   : Print the contents of a file.\n");
	printf("\tcp    : Copy file from host file system to Pure64 image.\n");
	printf("\tls    : List directory contents.\n");
	printf("\tmkdir : Create a directory.\n");
	printf("\tmkfs  : Create the file system image.\n");
	printf("\trm    : Remove a file.\n");
	printf("\trmdir : Remove a directory.\n");
}

static bool is_opt(const char *argv) {
	if (argv[0] == '-')
		return true;
	else
		return false;
}

/* * * * * * * * * * * *
 * Pure64FS Declarations
 * * * * * * * * * * * */

static int ramfs_export(struct pure64_fs *fs, const char *filename) {

	int err;
	FILE *file;
	long int pos;
	uint64_t sector_count;
	unsigned char sector_count_buf[2];

	struct pure64_stream stream;

	file = fopen(filename, "wb");
	if (file == NULL) {
		fprintf(stderr, "Failed to open '%s'.\n", filename);
		return EXIT_FAILURE;
	}

	if (fwrite(mbr_data, 1, mbr_data_size, file) != mbr_data_size) {
		fprintf(stderr, "Failed to write MBR to '%s'.\n", filename);
		fclose(file);
		return EXIT_FAILURE;
	}

	err = fseek(file, 0x2000, SEEK_SET);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to Pure64 location.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	if (fwrite(pure64_data, 1, pure64_data_size, file) != pure64_data_size) {
		fprintf(stderr, "Failed to write Pure64 to '%s'.\n", filename);
		fclose(file);
		return EXIT_FAILURE;
	}

	err = fseek(file, 0x4000, SEEK_SET);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to file system location.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	pure64_stream_init(&stream);
	stream.data = file;
	stream.read = fstream_read;
	stream.write = fstream_write;
	stream.set_pos = fstream_set_pos;
	stream.get_pos = fstream_get_pos;

	err = pure64_fs_export(fs, &stream);
	if (err != 0) {
		fprintf(stderr, "Failed to export Pure64 file system.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	pos = ftell(file);
	if (pos == -1L) {
		fprintf(stderr, "Failed to get file position.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	if ((pos % 512) != 0) {
		fseek(file, (pos + (512 - (pos % 512))) - 1, SEEK_SET);
		fputc(0x00, file);
		sector_count = ((pos - 0x2000) + 512) / 512;
	}

	/* get ready to update the required
	 * number of sectors to read from the
	 * master boot record */

	/* ensure that the sector
	 * count can be contained
	 * within a 16-bit, unsigned
	 * integer.
	 * */

	if (sector_count > 0xffff) {
		fprintf(stderr, "Pure64 file system is too large.\n");
		return EXIT_FAILURE;
	}

	sector_count -= 1;

	/* encode as little-endian */

	sector_count_buf[0] = (unsigned char)((sector_count >> 0x00) & 0xff);
	sector_count_buf[1] = (unsigned char)((sector_count >> 0x08) & 0xff);

	/* go to the location in the
	 * master boot record that contains
	 * the sector count to read.
	 */

	if (fseek(file, 0x01d2, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to sector count offset.\n");
		return EXIT_FAILURE;
	}

	/* write the sector count to
	 * the master boot record.
	 */

	if (fwrite(sector_count_buf, 1, 2, file) != 2) {
		fprintf(stderr, "Failed to write sector count.\n");
		return EXIT_FAILURE;
	}

	fclose(file);

	return EXIT_SUCCESS;
}

static int ramfs_import(struct pure64_fs *fs, const char *filename) {

	int err;
	FILE *file;
	struct pure64_stream stream;

	file = fopen(filename, "rb");
	if (file == NULL ) {
		fprintf(stderr, "Failed to open '%s' for reading.\n", filename);
		return EXIT_FAILURE;
	}

	err = fseek(file, 0x4000, SEEK_SET);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to file system location.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	pure64_stream_init(&stream);
	stream.data = file;
	stream.read = fstream_read;
	stream.write = fstream_write;
	stream.set_pos = fstream_set_pos;
	stream.get_pos = fstream_get_pos;

	if (pure64_fs_import(fs, &stream) != 0) {
		fprintf(stderr, "Failed to read file system from '%s'.\n", filename);
		fclose(file);
		return EXIT_FAILURE;
	}

	fclose(file);

	return EXIT_SUCCESS;
}

/* * * * * * * * * * * *
 * Command Declarations
 * * * * * * * * * * * */

static int pure64_init(const char *filename, int argc, const char **argv) {

	int err;
	FILE *file;
	const char *disk_uuid_str;
	int i;
	unsigned long long int disk_size;
	uint64_t minimum_size;
	struct gpt_header gpt_header;

	disk_uuid_str = NULL;
	disk_size = 1 * 1024 * 1024;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--disk-uuid") == 0) {
			disk_uuid_str = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--disk-size") == 0) {
			if ((i + 1) >= argc) {
				fprintf(stderr, "Disk size not specified.\n");
				return EXIT_FAILURE;
			} else if (sscanf(argv[i + 1], "%llu", &disk_size) != 0) {
				fprintf(stderr, "Malformed disk size was given");
				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr, "Unknown option '%s'\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	/* minimum gpt disk contains: */

	/* one gpt header */
	minimum_size = GPT_HEADER_SIZE;
	/* 128 primary partition headers */
	minimum_size += GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE;
	/* at least one sector per partition */
	minimum_size += GPT_PARTITION_HEADER_COUNT * 512;
	/* GPT_PARTITION_HEADER_COUNT backup partition headers */
	minimum_size += GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE;
	/* one backup gpt header */
	minimum_size += GPT_HEADER_SIZE;

	if (disk_size < minimum_size) {
		fprintf(stderr, "Disk size must be at least %u bytes.\n", 92U * 2U);
		return EXIT_FAILURE;
	}

	/* pad disk to nearest sector */

	if ((disk_size % 512) != 0) {
		disk_size += (512 - (disk_size % 512));
	}

	if (disk_uuid_str == NULL) {
		fprintf(stderr, "Warning: Disk UUID not specified.\n");
		disk_uuid_str = PURE64_DEFAULT_DISK_UUID;
	}

	err = pure64_uuid_parse(&gpt_header.disk_uuid, disk_uuid_str);
	if (err != 0) {
		fprintf(stderr, "Malformed disk UUID string.\n");
		return EXIT_FAILURE;
	}

	file = fopen(filename, "wb+");
	if (file == NULL) {
		fprintf(stderr, "Failed to open '%s' for writing.\n", filename);
		return EXIT_FAILURE;
	}

	if (fwrite(mbr_data, 1, mbr_data_size, file) != 512) {
		fprintf(stderr, "Failed to write multiboot record.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	/* The following code writes a GPT header
	 * to the file */

	/* TODO : swap little endian values if required */

	gpt_header.current_lba = 1;

	gpt_header.backup_lba = (disk_size - 512) / 512;

	/* mbr + gpt header + partition headers */
	gpt_header.first_usable_lba = (1 + 1) * 512;
	gpt_header.first_usable_lba += GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE;
	gpt_header.first_usable_lba /= 512;

	/* the backup lba minus 64 backup partition headers */
	gpt_header.last_usable_lba = (gpt_header.backup_lba * 512);
	gpt_header.last_usable_lba -= (GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE);
	gpt_header.last_usable_lba -= 512;
	gpt_header.last_usable_lba /= 512;

	gpt_header.partition_headers_lba = 2;

	gpt_header.partition_header_count = GPT_PARTITION_HEADER_COUNT;

	err = export_gpt_header(file, &gpt_header);
	if (err != 0) {
		fprintf(stderr, "Failed to export primary GPT header.\n");
		return EXIT_FAILURE;
	}

	/* zero primary partition headers */

	zero_file_data(file, GPT_PARTITION_HEADER_SIZE * GPT_PARTITION_HEADER_COUNT);

	/* zero backup partition headers */

	if (fseek(file, (gpt_header.backup_lba * 512) - (GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE), SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to the backup partition header array.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	zero_file_data(file, GPT_PARTITION_HEADER_SIZE * GPT_PARTITION_HEADER_COUNT);

	/* setup the backup header */

	if (fseek(file, gpt_header.backup_lba * 512, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to backup GPT header.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	/* update the information for the backup header */

	gpt_header.partition_headers_lba = gpt_header.backup_lba * 512;
	gpt_header.partition_headers_lba -= GPT_PARTITION_HEADER_COUNT * GPT_PARTITION_HEADER_SIZE;
	gpt_header.partition_headers_lba -= 512;
	gpt_header.partition_headers_lba /= 512;
	gpt_header.current_lba = gpt_header.backup_lba;
	gpt_header.backup_lba = 1;

	err = export_gpt_header(file, &gpt_header);
	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to export backup GPT header.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	err = calculate_checksums(file);
	if (err != EXIT_SUCCESS) {
		fclose(file);
		return err;
	}

	fclose(file);

	return EXIT_SUCCESS;
}

static int pure64_cat(struct pure64_fs *fs, int argc, const char **argv) {

	struct pure64_file *file;

	for (int i = 0; i < argc; i++) {

		file = pure64_fs_open_file(fs, argv[i]);
		if (file == NULL) {
			fprintf(stderr, "Failed to open '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}

		fwrite(file->data, 1, file->data_size, stdout);
	}

	return EXIT_SUCCESS;
}

static int pure64_cp(struct pure64_fs *fs, int argc, const char **argv) {

	int err;
	struct pure64_file *dst;
	FILE *src;
	long int src_size;
	const char *dst_path;
	const char *src_path;

	if (argc <= 0) {
		fprintf(stderr, "Missing source path.\n");
		return EXIT_FAILURE;
	} else if (argc <= 1) {
		fprintf(stderr, "Missing destination path.\n");
		return EXIT_FAILURE;
	}

	src_path = argv[0];
	dst_path = argv[1];

	src = fopen(src_path, "rb");
	if (src == NULL) {
		fprintf(stderr, "Failed to open source file '%s'.\n", src_path);
		return EXIT_FAILURE;
	}

	err = 0;

	err |= fseek(src, 0L, SEEK_END);

	src_size = ftell(src);

	err |= fseek(src, 0L, SEEK_SET);

	if ((err != 0) || (src_size < 0)) {
		fprintf(stderr, "Failed to get file size of '%s'.\n", src_path);
		fclose(src);
		return EXIT_FAILURE;
	}

	err = pure64_fs_make_file(fs, dst_path);
	if (err != 0) {
		fprintf(stderr, "Failed to create destination file '%s'.\n", dst_path);
		fclose(src);
		return EXIT_FAILURE;
	}

	dst = pure64_fs_open_file(fs, dst_path);
	if (dst == NULL) {
		fprintf(stderr, "Failed to open destination file '%s'.\n", dst_path);
		fclose(src);
		return EXIT_FAILURE;
	}

	dst->data = malloc(src_size);
	if (dst->data == NULL) {
		fprintf(stderr, "Failed to allocate memory for destination file '%s'.\n", dst_path);
		fclose(src);
		return EXIT_FAILURE;
	}

	if (fread(dst->data, 1, src_size, src) != ((size_t) src_size)) {
		fprintf(stderr, "Failed to read source file'%s'.\n", src_path);
		fclose(src);
		return EXIT_FAILURE;
	}

	fclose(src);

	dst->data_size = src_size;

	return EXIT_SUCCESS;
}

static int pure64_ls(struct pure64_fs *fs, int argc, const char **argv) {

	struct pure64_dir *subdir;

	if (argc == 0) {
		const char *default_args[] = { "/", NULL };
		return pure64_ls(fs, 1,  default_args);
	}

	for (int i = 0; i < argc; i++) {

		subdir = pure64_fs_open_dir(fs, argv[i]);
		if (subdir == NULL) {
			fprintf(stderr, "Failed to open '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}

		printf("%s:\n", argv[i]);

		for (uint64_t j = 0; j < subdir->subdir_count; j++)
			printf("dir  : %s\n", subdir->subdirs[j].name);

		for (uint64_t j = 0; j < subdir->file_count; j++)
			printf("file : %s\n", subdir->files[j].name);
	}

	return EXIT_SUCCESS;
}

static int pure64_mkdir(struct pure64_fs *fs, int argc, const char **argv) {

	int err;

	for (int i = 0; i < argc; i++) {
		err = pure64_fs_make_dir(fs, argv[i]);
		if (err != 0) {
			fprintf(stderr, "Failed to create directory '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int pure64_mkfs(const char *filename, int argc, const char **argv) {

	int err;
	struct pure64_fs fs;

	/* no arguments are currently
	 * needed for this command. */
	(void) argc;
	(void) argv;

	pure64_fs_init(&fs);

	err = ramfs_export(&fs, filename);
	if (err != EXIT_SUCCESS) {
		pure64_fs_free(&fs);
		return EXIT_FAILURE;
	}

	pure64_fs_free(&fs);

	return EXIT_SUCCESS;
}

int main(int argc, const char **argv) {

	int i;
	int err;
	const char *filename = "pure64.img";
	struct pure64_fs fs;

	for (i = 1; i < argc; i++) {
		if (check_opt(argv[i], "help", 'h')) {
			print_help(argv[0]);
			return EXIT_FAILURE;
		} else if (check_opt(argv[i], "file", 'f')) {
			filename = argv[i + 1];
			i++;
		} else if (is_opt(argv[i])) {
			fprintf(stderr, "Unknown option '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		} else {
			break;
		}
	}

	if (filename == NULL) {
		fprintf(stderr, "No filename specified after '--file' or '-f' option.\n");
		return EXIT_FAILURE;
	}

	if (i >= argc) {
		fprintf(stderr, "No command specified (see '--help').\n");
		return EXIT_FAILURE;
	}

	/* argv[i] should now point to a command. */

	if (strcmp(argv[i], "init") == 0) {
		return pure64_init(filename, argc - (i + 1), &argv[i + 1]);
	} else if (strcmp(argv[i], "mkfs") == 0) {
		return pure64_mkfs(filename, argc - (i + 1), &argv[i + 1]);
	}

	pure64_fs_init(&fs);

	err = ramfs_import(&fs, filename);
	if (err != EXIT_SUCCESS) {
		pure64_fs_free(&fs);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[i], "cat") == 0) {
		err = pure64_cat(&fs, argc - (i + 1), &argv[i + 1]);
	} else if (strcmp(argv[i], "cp") == 0) {
		err = pure64_cp(&fs, argc - (i + 1), &argv[i + 1]);
	} else if (strcmp(argv[i], "ls") == 0) {
		err = pure64_ls(&fs, argc - (i + 1), &argv[i + 1]);
	} else if (strcmp(argv[i], "mkdir") == 0) {
		err = pure64_mkdir(&fs, argc - (i + 1), &argv[i + 1]);
	} else if (strcmp(argv[i], "rm") == 0) {
	} else if (strcmp(argv[i], "rmdir") == 0) {
	} else {
		fprintf(stderr, "Unknown command '%s'.\n", argv[i]);
		pure64_fs_free(&fs);
		return EXIT_FAILURE;
	}

	if (err != EXIT_SUCCESS) {
		pure64_fs_free(&fs);
		return EXIT_FAILURE;
	}

	err = ramfs_export(&fs, filename);
	if (err != EXIT_SUCCESS) {
		pure64_fs_free(&fs);
		return EXIT_FAILURE;
	}

	pure64_fs_free(&fs);

	return EXIT_SUCCESS;
}
