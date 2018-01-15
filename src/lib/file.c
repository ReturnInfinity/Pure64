/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/file.h>
#include <pure64/error.h>
#include <pure64/stream.h>

#include "misc.h"

#include <stdlib.h>
#include <string.h>

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

int pure64_file_export(struct pure64_file *file, struct pure64_stream *out) {

	int err;

	err = encode_uint64(file->name_size, out);
	if (err != 0)
		return err;

	err = encode_uint64(file->data_size, out);
	if (err != 0)
		return err;

	err = pure64_stream_write(out, file->name, file->name_size);
	if (err != 0)
		return err;

	err = pure64_stream_write(out, file->data, file->data_size);
	if (err != 0)
		return err;

	return 0;
}

int pure64_file_import(struct pure64_file *file, struct pure64_stream *in) {

	int err;

	err = decode_uint64(&file->name_size, in);
	if (err != 0)
		return err;

	err = decode_uint64(&file->data_size, in);
	if (err != 0)
		return err;

	file->name = malloc(file->name_size + 1);
	file->data = malloc(file->data_size);
	if ((file->name == NULL) || (file->data == NULL)) {
		free(file->name);
		free(file->data);
		return -1;
	}

	err = pure64_stream_read(in, file->name, file->name_size);
	if (err != 0)
		return err;

	file->name[file->name_size] = 0;

	err = pure64_stream_read(in, file->data, file->data_size);
	if (err != 0)
		return err;

	return 0;
}

int pure64_file_set_name(struct pure64_file *file, const char *name) {

	char *tmp_name;
	unsigned long int name_size;

	name_size = strlen(name);

	tmp_name = malloc(name_size + 1);
	if (tmp_name == NULL) {
		return -1;
	}

	memcpy(tmp_name, name, name_size);

	tmp_name[name_size] = 0;

	free(file->name);

	file->name = tmp_name;
	file->name_size = name_size;

	return 0;
}
