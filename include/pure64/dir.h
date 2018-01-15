/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file */

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

/** Initializes a directory structure.
 * @param dir An uninitialized directory structure.
 * */

void pure64_dir_init(struct pure64_dir *dir);

/** Releases memory allocated by a directory.
 * @param dir An initialized directory structure.
 * */

void pure64_dir_free(struct pure64_dir *dir);

/** Serializes the directory to a stream.
 * @param dir An initialized directory structure.
 * @param out The stream to export the directory to.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_dir_export(struct pure64_dir *dir, FILE *out);

/** Deserializes a directory from a stream.
 * @param dir An initialized directory structure.
 * @param in The stream to read the directory from.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_dir_import(struct pure64_dir *dir, FILE *in);

/** Adds a file to the directory.
 * This function will fail if the name of the file exists.
 * @param dir An initialized directory structure.
 * @param name The name of the file.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_dir_add_file(struct pure64_dir *dir, const char *name);

/** Adds a subdirectory to the directory.
 * This function will fail if the name of the file exists.
 * @param dir An initialized directory structure.
 * @param name The name of the subdirectory.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_dir_add_subdir(struct pure64_dir *dir, const char *name);

/** Checks if a name exists in the directory as either a
 * file or a directory.
 * @param dir An initialized directory.
 * @param name The name to check for.
 * @returns True if the name is found, false if it is not.
 * */

bool pure64_dir_name_exists(const struct pure64_dir *dir, const char *name);

/** Sets the name of the directory.
 * @param dir An initialized directory.
 * @param name The new name of the directory.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_dir_set_name(struct pure64_dir *dir, const char *name);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_DIR_H */
