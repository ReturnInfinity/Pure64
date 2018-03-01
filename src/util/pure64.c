/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/fs.h>
#include <pure64/file.h>
#include <pure64/error.h>
#include <pure64/gpt.h>
#include <pure64/mbr.h>
#include <pure64/stream.h>
#include <pure64/uuid.h>

#include "mbr-data.h"
#include "pure64-data.h"
#include "stage-three-data.h"
#include "fstream.h"

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

#ifndef PURE64_DISK_LOCATION
#define PURE64_DISK_LOCATION (PURE64_FS_SECTOR * 512)
#endif

#ifndef PURE64_MINIMUM_DISK_SIZE
#define PURE64_MINIMUM_DISK_SIZE (1 * 1024 * 1024)
#endif

#ifndef PURE64_DEFAULT_DISK_UUID
#define PURE64_DEFAULT_DISK_UUID "74a7c14a-711d-4293-a731-569ca656799e"
#endif

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
	uint64_t st2_offset;
	uint64_t st3_offset;
	uint64_t fs_offset;
	struct pure64_mbr mbr;
	struct pure64_fstream fstream;

	/* Check that the 2nd and 3rd stage
	 * boot loaders aren't too big for
	 * the BIOS function to read from. */

	/* 0x7f is the maximum number of sectors that
	 * the BIOS function can read. Make sure that
	 * this limit is not exceeded. */

	if (((pure64_data_size + 511) / 512) > 0x7f) {
		fprintf(stderr, "2nd stage boot loader exceeds size limit.\n");
		return EXIT_FAILURE;
	}

	if (((stage_three_data_size + 511) / 512) > 0x7f) {
		fprintf(stderr, "2nd stage boot loader exceeds size limit.\n");
		return EXIT_FAILURE;
	}

	file = fopen(filename, "wb+");
	if (file == NULL) {
		fprintf(stderr, "Failed to open '%s'.\n", filename);
		return EXIT_FAILURE;
	}

	/* Write the master boot record to the
	 * beginning of the file.
	 * */

	if (fwrite(mbr_data, 1, mbr_data_size, file) != mbr_data_size) {
		fprintf(stderr, "Failed to write MBR to '%s'.\n", filename);
		fclose(file);
		return EXIT_FAILURE;
	}

	/* Set the locations of the boot loader
	 * stages and file system. */

	st2_offset = 0x2000;
	st3_offset = 0x2000 + pure64_data_size;
	fs_offset = st3_offset + stage_three_data_size;

	/* Round them off to the nearest sector. */

	st2_offset = ((st2_offset + 511) / 512) * 512;
	st3_offset = ((st3_offset + 511) / 512) * 512;
	fs_offset = ((fs_offset + 511) / 512) * 512;

	/* Seek to the location of the 2nd
	 * stage boot loader. */

	err = fseek(file, st2_offset, SEEK_SET);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to Pure64 location.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	/* Write the second stage boot loader. */

	if (fwrite(pure64_data, 1, pure64_data_size, file) != pure64_data_size) {
		fprintf(stderr, "Failed to write Pure64 to '%s'.\n", filename);
		fclose(file);
		return EXIT_FAILURE;
	}

	/* Seek to the location of the 3rd
	 * stage boot loader. */

	err = fseek(file, st3_offset, SEEK_SET);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to 3rd stage boot loader location.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	/* Write the third stage boot loader. */

	if (fwrite(stage_three_data, 1, stage_three_data_size, file) != stage_three_data_size) {
		fprintf(stderr, "Failed to write the third stage boot loader.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	/* Seek to the location of the file system. */

	err = fseek(file, fs_offset, SEEK_SET);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to file system location.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	/** Write the file system to the
	 * specific location.
	 * */

	pure64_fstream_init(&fstream);

	fstream.base.data = file;
	fstream.file = file;

	err = pure64_fs_export(fs, &fstream.base);
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

	if (pos < PURE64_MINIMUM_DISK_SIZE) {
		fseek(file, PURE64_MINIMUM_DISK_SIZE -1, SEEK_SET);
		fputc(0x00, file);
	} else if ((pos % 512) != 0) {
		fseek(file, (pos + (512 - (pos % 512))) - 1, SEEK_SET);
		fputc(0x00, file);
	}

	/* Update the MBR so that it knows where to find
	 * the 2nd and 3rd stage boot loaders. */

	pure64_mbr_zero(&mbr);

	err = pure64_mbr_read(&mbr, &fstream.base);
	if (err != 0) {
		fprintf(stderr, "Failed to read MBR: %s\n", pure64_strerror(err));
		fclose(file);
		return EXIT_FAILURE;
	}

	mbr.st2dap.sector = st2_offset / 512;
	mbr.st2dap.sector_count = (pure64_data_size + 511) / 512;
	mbr.st3dap.sector = st3_offset / 512;
	mbr.st3dap.sector_count = (stage_three_data_size + 511) / 512;

	err = pure64_mbr_write(&mbr, &fstream.base);
	if (err != 0) {
		fprintf(stderr, "Failed to write MBR: %s\n", pure64_strerror(err));
		fclose(file);
		return EXIT_FAILURE;
	}

	fclose(file);

	return EXIT_SUCCESS;
}

static int ramfs_import(struct pure64_fs *fs, const char *filename) {

	struct pure64_fstream fstream;

	pure64_fstream_init(&fstream);

	int err = pure64_fstream_open(&fstream, filename, "rb");
	if (err != 0) {
		fprintf(stderr, "Failed to open '%s' for reading.\n", filename);
		pure64_fstream_done(&fstream);
		return EXIT_FAILURE;
	}

	err = pure64_stream_set_pos(&fstream.base, PURE64_DISK_LOCATION);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to file system location.\n");
		pure64_fstream_done(&fstream);
		return EXIT_FAILURE;
	}

	if (pure64_fs_import(fs, &fstream.base) != 0) {
		fprintf(stderr, "Failed to read file system from '%s'.\n", filename);
		pure64_fstream_done(&fstream);
		return EXIT_FAILURE;
	}

	pure64_fstream_done(&fstream);

	return EXIT_SUCCESS;
}

/* * * * * * * * * * * *
 * Command Declarations
 * * * * * * * * * * * */

static int pure64_init(const char *filename, int argc, const char **argv) {

	int err;
	const char *disk_uuid_str;
	struct pure64_uuid disk_uuid;
	int i;
	unsigned long long int fs_size;
	uint64_t disk_size;
	struct pure64_mbr mbr;
	struct pure64_gpt gpt;
	struct pure64_fstream disk;

	disk_uuid_str = NULL;
	fs_size = 32 * 1024 * 1024;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--disk-uuid") == 0) {
			disk_uuid_str = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--fs-size") == 0) {
			if ((i + 1) >= argc) {
				fprintf(stderr, "File system size not specified.\n");
				return EXIT_FAILURE;
			} else if (sscanf(argv[i + 1], "%llu", &fs_size) != 0) {
				fprintf(stderr, "Malformed file system: %s\n", argv[i + 1]);
				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr, "Unknown option '%s'\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	/* pad file system size to nearest sector */

	fs_size = ((fs_size + 511) / 512) * 512;

	if (disk_uuid_str == NULL) {
		fprintf(stderr, "Warning: Disk UUID not specified.\n");
		disk_uuid_str = PURE64_DEFAULT_DISK_UUID;
	}

	err = pure64_uuid_parse(&disk_uuid, disk_uuid_str);
	if (err != 0) {
		fprintf(stderr, "Malformed disk UUID string.\n");
		return EXIT_FAILURE;
	}

	/* Open the disk file */

	pure64_fstream_init(&disk);

	err = pure64_fstream_open(&disk, filename, "wb+");
	if (err != 0) {
		fprintf(stderr, "Failed to open '%s' for writing.\n", filename);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Resize the file based on the file system
	 * size and all the static data. */

	disk_size = PURE64_GPT_MINIMUM_SIZE;
	disk_size += pure64_data_size;
	disk_size += stage_three_data_size;
	disk_size += fs_size;

	err = pure64_fstream_resize(&disk, disk_size);
	if (err != 0) {
		fprintf(stderr, "Failed to resize '%s'.\n", filename);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Write the master boot record */

	err = pure64_stream_set_pos(&disk.base, 0);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to MBR.\n");
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_stream_write(&disk.base, mbr_data, mbr_data_size);
	if (err != 0) {
		fprintf(stderr, "Failed to write multiboot record.\n");
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Initialize the GPT structure */

	pure64_gpt_init(&gpt);

	pure64_gpt_set_disk_uuid(&gpt, &disk_uuid);

	/* Format the GPT structure */

	err = pure64_gpt_format(&gpt, disk_size);
	if (err != 0) {
		fprintf(stderr, "Failed to format GPT: %s\n", pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Initialize stage two partition */

	err = pure64_gpt_set_entry_type(&gpt, 0, PURE64_UUID_STAGE_TWO);
	if (err != 0) {
		fprintf(stderr, "Failed to set GPT entry type: %s\n", pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_gpt_set_entry_name(&gpt, 0, u"Pure64 Stage Two");
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_gpt_set_entry_size(&gpt, 0, pure64_data_size);
	if (err != 0) {
		fprintf(stderr, "Failed to set GPT entry #%u size: %s\n", 0, pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_stream_set_pos(&disk.base, gpt.primary_entries[0].first_lba * 512);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_stream_write(&disk.base, pure64_data, pure64_data_size);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Initialize stage three partition */

	err = pure64_gpt_set_entry_type(&gpt, 1, PURE64_UUID_STAGE_THREE);
	if (err != 0) {
		fprintf(stderr, "Failed to set GPT entry type: %s\n", pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_gpt_set_entry_name(&gpt, 1, u"Pure64 Stage Three");
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_gpt_set_entry_size(&gpt, 1, stage_three_data_size);
	if (err != 0) {
		fprintf(stderr, "Failed to set GPT entry #%u size: %s\n", 1, pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_stream_set_pos(&disk.base, gpt.primary_entries[1].first_lba * 512);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_stream_write(&disk.base, stage_three_data, stage_three_data_size);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Initialize file system partition */

	err = pure64_gpt_set_entry_type(&gpt, 2, PURE64_UUID_FILE_SYSTEM);
	if (err != 0) {
		fprintf(stderr, "Failed to set GPT entry type: %s\n", pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_gpt_set_entry_name(&gpt, 2, u"Pure64 File System");
	if (err != 0) {
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	err = pure64_gpt_set_entry_size(&gpt, 2, 4096);
	if (err != 0) {
		fprintf(stderr, "Failed to set GPT entry #%u size: %s\n", 2, pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Export the GPT structure to disk */

	err = pure64_gpt_export(&gpt, &disk.base);
	if (err != 0) {
		fprintf(stderr, "Failed to export GPT to '%s': %s\n", filename, pure64_strerror(err));
		pure64_gpt_done(&gpt);
		pure64_fstream_done(&disk);
		return EXIT_FAILURE;
	}

	/* Update the MBR to load to the second
	 * and third stage boot loaders properly. */

	err = pure64_stream_set_pos(&disk.base, 0x00);
	if (err != 0) {
		fprintf(stderr, "Failed to seek to MBR: %s\n", pure64_strerror(err));
		pure64_fstream_done(&disk);
		pure64_gpt_done(&gpt);
		return EXIT_FAILURE;
	}

	pure64_mbr_zero(&mbr);

	err = pure64_mbr_read(&mbr, &disk.base);
	if (err != 0) {
		fprintf(stderr, "Failed to read MBR: %s\n", pure64_strerror(err));
		pure64_fstream_done(&disk);
		pure64_gpt_done(&gpt);
		return EXIT_FAILURE;
	}

	mbr.st2dap.sector = gpt.primary_entries[0].first_lba;
	mbr.st2dap.sector_count = (pure64_data_size + 511) / 512;
	mbr.st3dap.sector = gpt.primary_entries[1].first_lba;
	mbr.st3dap.sector_count = (stage_three_data_size + 511) / 512;

	err = pure64_mbr_write(&mbr, &disk.base);
	if (err != 0) {
		fprintf(stderr, "Failed to write MBR: %s\n", pure64_strerror(err));
		pure64_fstream_done(&disk);
		pure64_gpt_done(&gpt);
		return EXIT_FAILURE;
	}

	/* Release memory and exit. */

	pure64_gpt_done(&gpt);

	pure64_fstream_done(&disk);

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
		fprintf(stderr, "Failed to create destination file '%s': %s.\n", dst_path, pure64_strerror(err));
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
