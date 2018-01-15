/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file */

#ifndef PURE64_FS_H
#define PURE64_FS_H

#include <pure64/dir.h>

/** The correct value of a Pure64 file
 * system signature. This is a hexidecimal
 * version of the string "Pure64FS".
 * */

#define PURE64_SIGNATURE 0x5346343665727550

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_file;

/** Pure64 file system.
 * Used for storing kernels and
 * various files that assist in
 * the loading and execution of
 * an operating system.
 * */

struct pure64_fs {
	/** File system signature (see @ref PURE64_SIGNATURE). */
	uint64_t signature;
	/** The number of bytes occupied by the file system. This
	 * is useful for determining the amount of memory needed
	 * to load the file system. This value is calculed only
	 * when the file system is exported. */
	uint64_t size;
	/** The root directory of the
	 * file system. */
	struct pure64_dir root;
};

/** Initializes a file system structure.
 * @param fs An uninitialized file system structure.
 * */

void pure64_fs_init(struct pure64_fs *fs);

/** Releases resources allocated by the file
 * system structure.
 * @param fs An initialized file system structure.
 * */

void pure64_fs_free(struct pure64_fs *fs);

/** Exports the file system to a stream.
 * @param fs An initialized file system structure.
 * @param out The stream to export the file system to.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_fs_export(struct pure64_fs *fs, FILE *out);

/** Imports the file system from a stream.
 * @param fs An initialized file system structure.
 * @param in The stream to import the file system from.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_fs_import(struct pure64_fs *fs, FILE *in);

/** Creates a file in the file system.
 * @param fs An initialized file system structure.
 * @param path The path of the file to create.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_fs_make_file(struct pure64_fs *fs, const char *path);

/** Creates a subdirectory.
 * @param fs An initialized file system structure.
 * @param path The path to the directory to create.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_fs_make_dir(struct pure64_fs *fs, const char *path);

/** Opens an existing file.
 * @param fs An initialized file system structure.
 * @param path The path of the file to open.
 * @returns Zero on success, non-zero on failure.
 * */

struct pure64_file *pure64_fs_open_file(struct pure64_fs *fs, const char *path);

/** Opens an existing directory.
 * @param fs An initialized file system structure.
 * @param path The path of the directory to open.
 * @returns Zero on success, non-zero on failure.
 * */

struct pure64_dir *pure64_fs_open_dir(struct pure64_fs *fs, const char *path);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_FS_H */
