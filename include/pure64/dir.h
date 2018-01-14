/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_DIR_H
#define PURE64_DIR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_file;

/** A directory in a Pure64 image.
 * */

struct pure64_dir {
	/** The number of characters in the directory name. */
	uint64_t name_size;
	/** The number of subdirectories in the directory. */
	uint64_t subdir_count;
	/** The number o files in the directory. */
	uint64_t file_count;
	/** The directory name. */
	char *name;
	/** The subdirectories in the directory. */
	struct pure64_dir *subdirs;
	/** The files in the directory. */
	struct pure64_file *files;
};

void pure64_dir_init(struct pure64_dir *dir);

void pure64_dir_free(struct pure64_dir *dir);

int pure64_dir_export(struct pure64_dir *dir, FILE *out);

int pure64_dir_import(struct pure64_dir *dir, FILE *in);

int pure64_dir_add_file(struct pure64_dir *dir, const char *name);

int pure64_dir_add_subdir(struct pure64_dir *dir, const char *name);

int pure64_dir_make_file(struct pure64_dir *dir, const char *path);

int pure64_dir_make_subdir(struct pure64_dir *dir, const char *path);

struct pure64_file *pure64_dir_open_file(struct pure64_dir *dir, const char *path);

struct pure64_dir *pure64_dir_open_subdir(struct pure64_dir *dir, const char *path);

bool pure64_dir_name_exists(const struct pure64_dir *dir, const char *name);

int pure64_dir_set_name(struct pure64_dir *dir, const char *name);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_DIR_H */
