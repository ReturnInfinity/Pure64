/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/fs.h>
#include <pure64/file.h>
#include <pure64/path.h>
#include <pure64/error.h>
#include <pure64/string.h>

#include "misc.h"

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

static uint64_t pure64_file_size(const struct pure64_file *file) {
	return 16 + file->name_size + file->data_size;
}

static uint64_t pure64_dir_size(const struct pure64_dir *dir) {

	uint64_t size = 24;

	for (uint64_t i = 0; i < dir->subdir_count; i++)
		size += pure64_dir_size(&dir->subdirs[i]);

	for (uint64_t i = 0; i < dir->file_count; i++)
		size += pure64_file_size(&dir->files[i]);

	return size;
}

static uint64_t pure64_fs_size(const struct pure64_fs *fs) {

	return 16 + pure64_dir_size(&fs->root);
}

void pure64_fs_init(struct pure64_fs *fs) {
	fs->signature = PURE64_SIGNATURE;
	fs->size = 0;
	pure64_dir_init(&fs->root);
}

void pure64_fs_free(struct pure64_fs *fs) {
	pure64_dir_free(&fs->root);
}

int pure64_fs_export(struct pure64_fs *fs, struct pure64_stream *out) {

	int err;

	fs->size = pure64_fs_size(fs);

	err = encode_uint64(fs->signature, out);
	if (err != 0)
		return err;

	err = encode_uint64(fs->size, out);
	if (err != 0)
		return err;

	err = pure64_dir_export(&fs->root, out);
	if (err != 0)
		return err;

	return 0;
}

int pure64_fs_import(struct pure64_fs *fs, struct pure64_stream *in) {

	int err;

	err = decode_uint64(&fs->signature, in);
	if (err != 0)
		return err;

	if (fs->signature != PURE64_SIGNATURE)
		return PURE64_EINVAL;

	err = decode_uint64(&fs->size, in);
	if (err != 0)
		return err;

	err = pure64_dir_import(&fs->root, in);
	if (err != 0)
		return err;

	return 0;
}

int pure64_fs_make_dir(struct pure64_fs *fs, const char *path_str) {

	int err;
	const char *name;
	unsigned int name_count;
	unsigned int subdir_count;
	unsigned int i;
	unsigned int j;
	struct pure64_path path;
	struct pure64_dir *parent_dir;
	struct pure64_dir *subdir;

	pure64_path_init(&path);

	err = pure64_path_parse(&path, path_str);
	if (err != 0) {
		pure64_path_free(&path);
		return err;
	}

	err = pure64_path_normalize(&path);
	if (err != 0) {
		pure64_path_free(&path);
		return err;
	}

	parent_dir = &fs->root;

	name_count = pure64_path_get_name_count(&path);

	if (name_count == 0) {
		pure64_path_free(&path);
		return PURE64_EINVAL;
	}

	for (i = 0; i < (name_count - 1); i++) {

		name = pure64_path_get_name(&path, i);
		if (name == NULL) {
			pure64_path_free(&path);
			return PURE64_EFAULT;
		}

		subdir_count = parent_dir->subdir_count;

		for (j = 0; j < subdir_count; j++) {
			subdir = &parent_dir->subdirs[j];
			if (subdir == NULL) {
				continue;
			} else if (pure64_strcmp(subdir->name, name) == 0) {
				parent_dir = subdir;
				break;
			}
		}

		if (j >= subdir_count) {
			/* not found */
			pure64_path_free(&path);
			return PURE64_ENOENT;
		}
	}

	if (i != (name_count - 1)) {
		pure64_path_free(&path);
		return PURE64_ENOENT;
	}

	err = pure64_dir_add_subdir(parent_dir, path.name_array[i].data);
	if (err != 0) {
		pure64_path_free(&path);
		return err;
	}

	pure64_path_free(&path);

	return 0;
}

int pure64_fs_make_file(struct pure64_fs *fs, const char *path_str) {

	int err;
	const char *name;
	unsigned int name_count;
	unsigned int subdir_count;
	unsigned int i;
	unsigned int j;
	struct pure64_path path;
	struct pure64_dir *parent_dir;
	struct pure64_dir *subdir;

	pure64_path_init(&path);

	err = pure64_path_parse(&path, path_str);
	if (err != 0) {
		pure64_path_free(&path);
		return err;
	}

	err = pure64_path_normalize(&path);
	if (err != 0) {
		pure64_path_free(&path);
		return err;
	}

	parent_dir = &fs->root;

	name_count = pure64_path_get_name_count(&path);

	if (name_count == 0) {
		pure64_path_free(&path);
		return PURE64_EINVAL;
	}

	for (i = 0; i < (name_count - 1); i++) {

		name = pure64_path_get_name(&path, i);
		if (name == NULL) {
			pure64_path_free(&path);
			return PURE64_EINVAL;
		}

		subdir_count = parent_dir->subdir_count;

		for (j = 0; j < subdir_count; j++) {
			subdir = &parent_dir->subdirs[j];
			if (subdir == NULL) {
				continue;
			} else if (pure64_strcmp(subdir->name, name) == 0) {
				parent_dir = subdir;
				break;
			}
		}

		if (j >= subdir_count) {
			/* not found */
			pure64_path_free(&path);
			return PURE64_ENOENT;
		}
	}

	if (i != (name_count - 1)) {
		pure64_path_free(&path);
		return PURE64_ENOENT;
	}

	err = pure64_dir_add_file(parent_dir, path.name_array[i].data);
	if (err != 0) {
		pure64_path_free(&path);
		return err;
	}

	pure64_path_free(&path);

	return 0;
}

struct pure64_dir *pure64_fs_open_dir(struct pure64_fs *fs, const char *path_string) {

	int err;
	unsigned int i;
	unsigned int j;
	const char *name;
	unsigned int name_count;
	unsigned int subdir_count;
	struct pure64_path path;
	struct pure64_dir *parent_dir;

	pure64_path_init(&path);

	err = pure64_path_parse(&path, path_string);
	if (err != 0) {
		pure64_path_free(&path);
		return NULL;
	}

	err = pure64_path_normalize(&path);
	if (err != 0) {
		pure64_path_free(&path);
		return NULL;
	}

	parent_dir = &fs->root;

	name_count = pure64_path_get_name_count(&path);

	for (i = 0; i < name_count; i++) {
		name = pure64_path_get_name(&path, i);
		if (name == NULL) {
			pure64_path_free(&path);
			return NULL;
		}

		subdir_count = parent_dir->subdir_count;
		for (j = 0; j < subdir_count; j++) {
			if (pure64_strcmp(parent_dir->subdirs[j].name, name) == 0) {
				parent_dir = &parent_dir->subdirs[j];
				break;
			}
		}

		if (j >= subdir_count) {
			pure64_path_free(&path);
			return NULL;
		}
	}

	pure64_path_free(&path);

	return parent_dir;
}

struct pure64_file *pure64_fs_open_file(struct pure64_fs *fs, const char *path_string) {

	int err;
	unsigned int i;
	unsigned int j;
	const char *name;
	unsigned int subdir_count;
	unsigned int name_count;
	struct pure64_path path;
	struct pure64_dir *parent_dir;

	pure64_path_init(&path);

	err = pure64_path_parse(&path, path_string);
	if (err != 0) {
		pure64_path_free(&path);
		return NULL;
	}

	err = pure64_path_normalize(&path);
	if (err != 0) {
		pure64_path_free(&path);
		return NULL;
	}

	parent_dir = &fs->root;

	name_count = pure64_path_get_name_count(&path);

	if (name_count == 0) {
		/* there must be at least one
		 * entry name in the path */
		pure64_path_free(&path);
		return NULL;
	}

	for (i = 0; i < (name_count - 1); i++) {
		name = pure64_path_get_name(&path, i);
		if (name == NULL) {
			pure64_path_free(&path);
			return NULL;
		}

		subdir_count = parent_dir->subdir_count;
		for (j = 0; j < subdir_count; j++) {
			if (pure64_strcmp(parent_dir->subdirs[j].name, name) == 0) {
				parent_dir = &parent_dir->subdirs[j];
				break;
			}
		}

		if (j >= subdir_count) {
			pure64_path_free(&path);
			return NULL;
		}
	}

	name = pure64_path_get_name(&path, i);
	if (name == NULL) {
		/* This shouldn't happen, so
		 * this check is a precaution */
		pure64_path_free(&path);
		return NULL;
	}

	/* 'name' is now the basename of the file. */

	for (j = 0; j < parent_dir->file_count; j++) {
		if (pure64_strcmp(parent_dir->files[j].name, name) == 0) {
			pure64_path_free(&path);
			return &parent_dir->files[j];
		}
	}

	/* file not found */

	pure64_path_free(&path);

	return NULL;
}
