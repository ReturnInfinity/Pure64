/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dir.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

static int ramfs_export(struct pure64_dir *root, const char *filename) {

	int err;
	FILE *file;

	file = fopen(filename, "wb");
	if (file == NULL) {
		fprintf(stderr, "Failed to open '%s'.\n", filename);
		return EXIT_FAILURE;
	}

	err = pure64_dir_export(root, file);
	if (err != 0) {
		fprintf(stderr, "Failed to export RamFS file system.\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	fclose(file);

	return EXIT_SUCCESS;
}

static int mkfs(const char *filename, int argc, const char **argv) {

	int err;
	struct pure64_dir root;

	/* no arguments are currently
	 * needed for this command. */
	(void) argc;
	(void) argv;

	pure64_dir_init(&root);

	err = ramfs_export(&root, filename);
	if (err != EXIT_SUCCESS)
		return EXIT_FAILURE;

	pure64_dir_free(&root);

	return EXIT_SUCCESS;
}

int main(int argc, const char **argv) {

	int i;
	const char *filename = "pure64.img";

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
		return mkfs(filename, i - 1, &argv[i + 1]);
	}

	if (strcmp(argv[i], "cat") == 0) {
	} else if (strcmp(argv[i], "cp") == 0) {
	} else if (strcmp(argv[i], "ls") == 0) {
	} else if (strcmp(argv[i], "mkdir") == 0) {
	} else if (strcmp(argv[i], "rm") == 0) {
	} else if (strcmp(argv[i], "rmdir") == 0) {
	} else {
		fprintf(stderr, "Unknown command '%s'.\n", argv[i]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
