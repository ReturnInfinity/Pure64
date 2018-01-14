/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_FILE_H
#define PURE64_FILE_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A Pure64 file.
 * */

struct pure64_file {
	/** The number of characters in the file name. */
	uint64_t name_size;
	/** The number of bytes in the file data. */
	uint64_t data_size;
	/** The name of the file. */
	char *name;
	/** The file data. */
	void *data;
};

void pure64_file_init(struct pure64_file *file);

void pure64_file_free(struct pure64_file *file);

int pure64_file_export(struct pure64_file *file, FILE *out);

int pure64_file_import(struct pure64_file *file, FILE *in);

int pure64_file_set_name(struct pure64_file *file, const char *name);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_FILE_H */
