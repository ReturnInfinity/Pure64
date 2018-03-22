/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file file.h API related to files in the Pure64 file system. */

#ifndef PURE64_FILE_H
#define PURE64_FILE_H

#include <pure64/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_stream;

/** A Pure64 file.
 * */

struct pure64_file {
	/** The number of characters in the file name. */
	pure64_uint64 name_size;
	/** The number of bytes in the file data. */
	pure64_uint64 data_size;
	/** The name of the file. */
	char *name;
	/** The file data. */
	void *data;
};

/** Initializes a file structure.
 * @param file An uninitialized file structure.
 * */

void pure64_file_init(struct pure64_file *file);

/** Releases memory allocated by the file structure.
 * @param file An initialized file structure.
 * */

void pure64_file_free(struct pure64_file *file);

/** Serializes a file to a stream.
 * @param file An initialized file structure.
 * @param out The stream to export the file to.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_file_export(struct pure64_file *file, struct pure64_stream *out);

/** Deserializes a file from a stream.
 * @param file An initialized file structure.
 * @param in The stream to read the file from.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_file_import(struct pure64_file *file, struct pure64_stream *in);

/** Sets the name of the file.
 * @param file An initialized file structure.
 * @param name The new name of the file.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_file_set_name(struct pure64_file *file, const char *name);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_FILE_H */
