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

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

void *pure64_malloc(uint64_t size) {
	return malloc(size);
}

void *pure64_realloc(void *addr, uint64_t size) {
	return realloc(addr, size);
}

void pure64_free(void *addr) {
	free(addr);
}

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

	if (strcmp(argv[i], "mkfs") == 0) {
		return pure64_mkfs(filename, i - 1, &argv[i + 1]);
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
