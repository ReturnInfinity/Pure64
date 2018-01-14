/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/file.h>

#include "misc.h"

#include <stdlib.h>

void pure64_file_init(struct pure64_file *file) {
	file->name_size = 0;
	file->data_size = 0;
	file->name = NULL;
	file->data = NULL;
}

void pure64_file_free(struct pure64_file *file) {
	free(file->name);
	free(file->data);
	file->name = NULL;
	file->data = NULL;
}

int pure64_file_export(struct pure64_file *file, FILE *out) {

	int err = 0;
	err |= encode_uint64(file->name_size, out);
	err |= encode_uint64(file->data_size, out);
	if (err != 0)
		return -1;

	if (fwrite(file->name, 1, file->name_size, out) != file->name_size)
		return -1;

	if (fwrite(file->data, 1, file->data_size, out) != file->data_size)
		return -1;

	return 0;
}

int pure64_file_import(struct pure64_file *file, FILE *in) {

	int err = 0;
	err |= decode_uint64(&file->name_size, in);
	err |= decode_uint64(&file->data_size, in);
	if (err != 0)
		return -1;

	file->name = malloc(file->name_size + 1);
	file->data = malloc(file->data_size);
	if ((file->name == NULL) || (file->data == NULL)) {
		free(file->name);
		free(file->data);
		return -1;
	}

	if (fread(file->name, 1, file->name_size, in) != file->name_size)
		return -1;

	file->name[file->name_size] = 0;

	if (fread(file->data, 1, file->data_size, in) != file->data_size)
		return -1;

	return 0;
}
