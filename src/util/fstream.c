/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "fstream.h"

#include <pure64/error.h>

#include <limits.h>
#include <stdlib.h>

static int fstream_set_pos(void *file_ptr, uint64_t pos_ptr) {

	if (pos_ptr > LONG_MAX)
		return PURE64_EINVAL;

	if (fseek((FILE *) file_ptr, pos_ptr, SEEK_SET) != 0)
		return PURE64_EIO;

	return 0;
}

static int fstream_get_pos(void *file_ptr, uint64_t *pos_ptr) {

	long int pos;

	pos = ftell((FILE *) file_ptr);
	if (pos == -1L)
		return PURE64_EIO;

	*pos_ptr = pos;

	return 0;
}

static int fstream_get_size(void *file_ptr, uint64_t *size_ptr) {

	FILE *file = (FILE *) file_ptr;
	if (file == NULL)
		return PURE64_EFAULT;

	long int original_pos = ftell(file);
	if (original_pos == -1L)
		/* TODO : more specific error code */
		return PURE64_EINVAL;

	if (fseek(file, 0, SEEK_END) != 0)
		/* TODO : more specific error code */
		return PURE64_EINVAL;

	long int size = ftell(file);
	if (size == -1L)
		/* TODO : more specific error code */
		return PURE64_EINVAL;

	if (fseek(file, original_pos, SEEK_SET) != 0)
		/* TODO : more specific error code */
		return PURE64_EINVAL;

	if (size_ptr != NULL)
		*size_ptr = (uint64_t) size;

	return 0;
}

static int fstream_write(void *file_ptr, const void *buf, uint64_t buf_size) {
	if (fwrite(buf, 1, buf_size, (FILE *) file_ptr) != buf_size)
		return PURE64_EIO;
	else
		return 0;
}

static int fstream_read(void *file_ptr, void *buf, uint64_t buf_size) {

	if (file_ptr == NULL)
		return PURE64_EFAULT;

	if (fread(buf, 1, buf_size, (FILE *) file_ptr) != buf_size)
		return PURE64_EIO;
	else
		return 0;
}

void pure64_fstream_init(struct pure64_fstream *fstream) {
	pure64_stream_init(&fstream->base);
	fstream->base.data = NULL;
	fstream->base.set_pos = fstream_set_pos;
	fstream->base.get_pos = fstream_get_pos;
	fstream->base.get_size = fstream_get_size;
	fstream->base.write = fstream_write;
	fstream->base.read = fstream_read;
	fstream->file = NULL;
}

void pure64_fstream_done(struct pure64_fstream *fstream) {
	if (fstream->file != NULL) {
		fclose(fstream->file);
		fstream->file = NULL;
		fstream->base.data = NULL;
	}
}

int pure64_fstream_resize(struct pure64_fstream *fstream,
                          long int size) {

	if (size < 1)
		return PURE64_EINVAL;

	int err = pure64_stream_set_pos(&fstream->base, size - 1);
	if (err != 0)
		return err;

	err = pure64_stream_write(&fstream->base, "\x00", 1);
	if (err != 0)
		return err;

	return 0;
}

int pure64_fstream_open(struct pure64_fstream *fstream,
                        const char *path,
                        const char *mode) {

	FILE *file = fopen(path, mode);
	if (file == NULL) {
		/* TODO : more specific error code */
		return PURE64_ENOENT;
	}

	if (fstream->file != NULL)
		fclose(fstream->file);

	fstream->file = file;
	fstream->base.data = file;

	return 0;
}

int pure64_fstream_zero(struct pure64_fstream *fstream,
                        unsigned long int count) {

	unsigned long int buf_size = 4096;

	unsigned char *buf = calloc(1, buf_size);

	unsigned long int read_count = 0;

	while ((read_count + buf_size) < count) {

		int err = pure64_stream_write(&fstream->base, buf, buf_size);
		if (err != 0) {
			free(buf);
			return err;
		}

		read_count += buf_size;
	}

	int err = pure64_stream_write(&fstream->base, buf, count - read_count);
	if (err != 0) {
		free(buf);
		return err;
	}

	free(buf);

	return 0;
}
