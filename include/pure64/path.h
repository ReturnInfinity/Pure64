/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_PATH_H
#define PURE64_PATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A file or directory name that
 * is part of a path structure.
 * */

struct pure64_path_name {
	/** The name characters */
	char *data;
	/** The number of characters
	 * in the name. */
	uint64_t size;
} *name_array;

/** A file system path.
 * */

struct pure64_path {
	/** The directory name array */
	struct pure64_path_name *name_array;
	/** The number of names in the
	 * directory name array. */
	uint64_t name_count;
};

/** Initializes a path structure.
 * @param path An uninitialized
 * path structure.
 * */

void
pure64_path_init(struct pure64_path *path);

/** Releases resources allocated
 * by the path.
 * @param path An initialized path
 * structure.
 * */

void
pure64_path_free(struct pure64_path *path);

/** Get a directory name from the path.
 * @param path An initialized path structure.
 * @param index The index of the directory
 * name within the path to get.
 * @returns The name of the directory
 * within the path. If the index
 * is out of bounds, null is returned.
 * */

const char *
pure64_path_get_name(const struct pure64_path *path,
                     uint64_t index);

/** Get the number of directories specified in the path.
 * @param path An initialized path structure.
 * @returns The number of directories in the path.
 * */

uint64_t
pure64_path_get_name_count(const struct pure64_path *path);

/** Resolve '.' and '..' references.
 * If there are more '..' references than
 * there are real directory names, the root
 * directory will be leftover.
 * @param path An initialized path structure
 * @returns Zero on success, non-zero on failure.
 * */

int
pure64_path_normalize(struct pure64_path *path);

/** Parses a path string.
 * @param path An initialized path string.
 * @param path_string The string to parse
 * path from.
 * @returns Zero on success, non-zero on
 * failure.
 * */

int
pure64_path_parse(struct pure64_path *path,
                  const char *path_string);

/** Add a file or directory name to the
 * end of the path.
 * @param path An initialized path structure.
 * @param name The file or directory name to push.
 * @returns Zero on success, non-zero otherwise.
 * */

int
pure64_path_push_child(struct pure64_path *path,
                       const char *name);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_PATH_H */
