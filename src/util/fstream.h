/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_FSTREAM_H
#define PURE64_FSTREAM_H

#include <pure64/stream.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** This structure represents
 * a file stream. It is used to
 * format GPT images on a file,
 * as opposed to a disk.
 * */

struct pure64_fstream {
	/** The base stream structure */
	struct pure64_stream base;
	/** The file pointer for the
	 * stream. */
	FILE *file;
};

/** Initializes a file stream structure.
 * @param fstream The file stream structure
 * to initialize.
 * */

void pure64_fstream_init(struct pure64_fstream *fstream);

/** Closes the file, if there was one open.
 * @param fstream An initalized file stream.
 * */

void pure64_fstream_done(struct pure64_fstream *fstream);

/** Opens a file to be used in the file stream.
 * @param fstream An initialized file stream structure.
 * @param path The path of the file to open.
 * @param mode The mode to open the file in.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_fstream_open(struct pure64_fstream *fstream,
                        const char *path,
                        const char *mode);

/** Resize the file.
 * This function only works if
 * the file is open for writing.
 * @param fstream An initialized
 * file structure.
 * @param size The new size of
 * the file.
 * @returns Zero on success, an
 * error code on failure.
 * */

int pure64_fstream_resize(struct pure64_fstream *fstream,
                          long int size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_FSTREAM_H */
