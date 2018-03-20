/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "util.h"

#include <pure64/error.h>
#include <pure64/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	printf("\t--disk,   -d : Specify the path to the disk file.\n");
	printf("\t--config, -c : Specify the path to the config file.\n");
	printf("\t--help,   -h : Print this help message.\n");
	printf("\n");
	printf("Commands:\n");
	printf("\tinit  : Initialize the disk image.\n");
	printf("\tcat   : Print the contents of a file.\n");
	printf("\tcp    : Copy file from host file system to Pure64 image.\n");
	printf("\tls    : List directory contents.\n");
	printf("\tmkdir : Create a directory.\n");
}

static bool is_opt(const char *argv) {
	if (argv[0] == '-')
		return true;
	else
		return false;
}

/* * * * * * * * * * * *
 * Command Declarations
 * * * * * * * * * * * */

static int pure64_init(const char *config, const char *disk, int argc, const char **argv) {

	(void) argc;
	(void) argv;

	struct pure64_util util;

	pure64_util_init(&util);

	int err = pure64_util_open_config(&util, config);
	if (err != 0) {
		fprintf(stderr, "Failed to open config '%s': %s\n", config, pure64_strerror(err));
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	err = pure64_util_create_disk(&util, disk);
	if (err != 0) {
		fprintf(stderr, "Failed to create disk image: %s\n", pure64_strerror(err));
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	pure64_util_done(&util);

	return EXIT_SUCCESS;
}

static int pure64_ls(struct pure64_util *util, int argc, const char **argv) {

	struct pure64_fs *fs = &util->fs;

	if (argc == 0) {
		const char *default_args[] = { "/", NULL };
		return pure64_ls(util, 1,  default_args);
	}

	struct pure64_dir *subdir;

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

static int pure64_mkdir(struct pure64_util *util, int argc, const char **argv) {

	struct pure64_fs *fs = &util->fs;

	for (int i = 0; i < argc; i++) {
		int err = pure64_fs_make_dir(fs, argv[i]);
		if (err != 0) {
			fprintf(stderr, "Failed to create directory '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int pure64_cat(struct pure64_util *util, int argc, const char **argv) {

	struct pure64_fs *fs = &util->fs;

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

static int pure64_cp(struct pure64_util *util, int argc, const char **argv) {

	if (argc <= 0) {
		fprintf(stderr, "Missing source path.\n");
		return EXIT_FAILURE;
	} else if (argc <= 1) {
		fprintf(stderr, "Missing destination path.\n");
		return EXIT_FAILURE;
	}

	const char *src_path = argv[0];
	const char *dst_path = argv[1];

	FILE *src = fopen(src_path, "rb");
	if (src == NULL) {
		fprintf(stderr, "Failed to open source file '%s'.\n", src_path);
		return EXIT_FAILURE;
	}

	int err = fseek(src, 0L, SEEK_END);

	long int src_size = ftell(src);

	err |= fseek(src, 0L, SEEK_SET);

	if ((err != 0) || (src_size < 0)) {
		fprintf(stderr, "Failed to get file size of '%s'.\n", src_path);
		fclose(src);
		return EXIT_FAILURE;
	}

	struct pure64_fs *fs = &util->fs;

	err = pure64_fs_make_file(fs, dst_path);
	if (err != 0) {
		fprintf(stderr, "Failed to create destination file '%s': %s.\n", dst_path, pure64_strerror(err));
		fclose(src);
		return EXIT_FAILURE;
	}

	struct pure64_file *dst;

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

int main(int argc, const char **argv) {

	const char *disk = "pure64.img";

	const char *config = "pure64-config.txt";

	int i = 1;

	while (i < argc) {
		if (check_opt(argv[i], "help", 'h')) {
			print_help(argv[0]);
			return EXIT_FAILURE;
		} else if (check_opt(argv[i], "disk", 'd')) {
			disk = argv[i + 1];
			i += 2;
		} else if (check_opt(argv[i], "config", 'c')) {
			config = argv[i + 1];
			i += 2;
		} else if (is_opt(argv[i])) {
			fprintf(stderr, "Unknown option '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		} else {
			break;
		}
	}

	if (disk == NULL) {
		fprintf(stderr, "No disk specified after '--disk' or '-d' option.\n");
		return EXIT_FAILURE;
	}

	if (config == NULL) {
		fprintf(stderr, "No configuration file specified after '--config' or '-c' option.\n");
		return EXIT_FAILURE;
	}

	if (i >= argc) {
		fprintf(stderr, "No command specified (see '--help').\n");
		return EXIT_FAILURE;
	}

	argv = &argv[i];
	argc -= i;

	/* argv[0] should now point to a command. */

	const char *command = argv[0];

	argv = &argv[1];
	argc--;

	/* argv[0] is now the command arguments */

	if (strcmp(command, "init") == 0) {
		return pure64_init(config, disk, argc, argv);
	}

	struct pure64_util util;

	pure64_util_init(&util);

	int err = pure64_util_open_config(&util, config);
	if (err != 0) {
		fprintf(stderr, "Failed to open '%s': %s\n", config, pure64_strerror(err));
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	err = pure64_util_open_disk(&util, disk);
	if (err != 0) {
		fprintf(stderr, "Failed to open '%s': %s\n", disk, pure64_strerror(err));
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	int exit_code = EXIT_SUCCESS;

	if (strcmp(command, "cat") == 0) {
		exit_code = pure64_cat(&util, argc, argv);
	} else if (strcmp(command, "cp") == 0) {
		exit_code = pure64_cp(&util, argc, argv);
	} else if (strcmp(command, "ls") == 0) {
		exit_code = pure64_ls(&util, argc, argv);
	} else if (strcmp(command, "mkdir") == 0) {
		exit_code = pure64_mkdir(&util, argc, argv);
	} else {
		fprintf(stderr, "Unknown command '%s'.\n", command);
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	if (exit_code != EXIT_SUCCESS) {
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	err = pure64_util_save_disk(&util);
	if (err != 0) {
		fprintf(stderr, "Failed to save disk changes.\n");
		pure64_util_done(&util);
		return EXIT_FAILURE;
	}

	pure64_util_done(&util);

	return EXIT_SUCCESS;
}
