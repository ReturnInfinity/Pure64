/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dir.h>
#include <pure64/file.h>

#include "misc.h"

#include <stdlib.h>

void pure64_dir_init(struct pure64_dir *dir) {
	dir->name_size = 0;
	dir->subdir_count = 0;
	dir->file_count = 0;
	dir->name = NULL;
	dir->subdirs = NULL;
	dir->files = NULL;
}

void pure64_dir_free(struct pure64_dir *dir) {

	free(dir->name);

	for (uint64_t i = 0; i < dir->subdir_count; i++)
		pure64_dir_free(&dir->subdirs[i]);

	for (uint64_t i = 0; i < dir->file_count; i++)
		pure64_file_free(&dir->files[i]);

	dir->name = NULL;
	dir->subdirs = NULL;
	dir->files = NULL;
}

int pure64_dir_export(struct pure64_dir *dir, FILE *out) {

	int err = 0;
	err |= encode_uint64(dir->name_size, out);
	err |= encode_uint64(dir->subdir_count, out);
	err |= encode_uint64(dir->file_count, out);
	if (err != 0)
		return -1;

	if (fwrite(dir->name, 1, dir->name_size, out) != dir->name_size)
		return -1;

	for (uint64_t i = 0; i < dir->subdir_count; i++) {
		err = pure64_dir_export(&dir->subdirs[i], out);
		if (err != 0)
			return -1;
	}

	for (uint64_t i = 0; i < dir->file_count; i++) {
		err = pure64_file_export(&dir->files[i], out);
		if (err != 0)
			return -1;
	}

	return 0;
}

int pure64_dir_import(struct pure64_dir *dir, FILE *in) {

	int err = 0;
	err |= decode_uint64(&dir->name_size, in);
	err |= decode_uint64(&dir->subdir_count, in);
	err |= decode_uint64(&dir->file_count, in);
	if (err != 0)
		return -1;

	dir->name = malloc(dir->name_size + 1);
	dir->subdirs = malloc(dir->subdir_count * sizeof(dir->subdirs[0]));
	dir->files = malloc(dir->subdir_count * sizeof(dir->files[0]));
	if ((dir->name == NULL)
	 || (dir->subdirs == NULL)
	 || (dir->files == NULL)) {
		free(dir->name);
		free(dir->subdirs);
		free(dir->files);
		return -1;
	}

	if (fread(dir->name, 1, dir->name_size, in) != dir->name_size)
		return -1;

	dir->name[dir->name_size] = 0;

	for (uint64_t i = 0; i < dir->subdir_count; i++)
		pure64_dir_init(&dir->subdirs[i]);

	for (uint64_t i = 0; i < dir->file_count; i++)
		pure64_file_init(&dir->files[i]);

	for (uint64_t i = 0; i < dir->subdir_count; i++) {
		err = pure64_dir_import(&dir->subdirs[i], in);
		if (err != 0)
			return -1;
	}

	for (uint64_t i = 0; i < dir->file_count; i++) {
		err = pure64_file_import(&dir->files[i], in);
		if (err != 0)
			return -1;
	}

	return 0;
}
