/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "util.h"

#include "config.h"

#include "pure64-data.h"
#include "stage-three-data.h"

#include <pure64/error.h>
#include <pure64/gpt.h>

#include <stdlib.h>

struct kernel_buf {
	void *data;
	unsigned long int size;
};

static int kernel_open(struct kernel_buf *kernel_buf, const char *path) {

	kernel_buf->data = NULL;
	kernel_buf->size = 0;

	FILE *kernel = fopen(path, "rb");
	if (kernel == NULL)
		return PURE64_ENOENT;

	if (fseek(kernel, 0UL, SEEK_END) != 0) {
		fclose(kernel);
		return PURE64_EINVAL;
	}

	long int kernel_size = ftell(kernel);
	if (kernel_size == -1L) {
		fclose(kernel);
		return PURE64_EINVAL;
	}

	if (fseek(kernel, 0UL, SEEK_SET) != 0) {
		fclose(kernel);
		return PURE64_EINVAL;
	}

	void *buf = malloc(kernel_size);
	if (buf == NULL) {
		fclose(kernel);
		return PURE64_ENOMEM;
	}

	if (fread(buf, 1, kernel_size, kernel) != ((size_t) kernel_size)) {
		fclose(kernel);
		free(buf);
		return PURE64_EIO;
	}

	fclose(kernel);

	kernel_buf->data = buf;
	kernel_buf->size = (unsigned long int) kernel_size;

	return 0;
}

void kernel_done(struct kernel_buf *kernel) {
	free(kernel->data);
	kernel->data = NULL;
	kernel->size = 0;
}

static int write_bootsector(struct pure64_util *util) {

	const void *bootsector = pure64_bootsector_data(util->config.bootsector);

	unsigned long int bootsector_size = pure64_bootsector_size(util->config.bootsector);

	int err = pure64_stream_set_pos(&util->disk_file.base, 0);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, bootsector, bootsector_size);
	if (err != 0)
		return err;

	return 0;
}

static int write_kernel_bin(struct pure64_util *util) {

	const char *path = util->config.kernel;
	if (path == NULL)
		path = "kernel";

	struct kernel_buf kernel_buf;

	int err = kernel_open(&kernel_buf, path);
	if (err != 0) {
		kernel_done(&kernel_buf);
		return err;
	}

	unsigned long int kernel_offset = 0;
	kernel_offset += pure64_bootsector_size(util->config.bootsector);
	kernel_offset += pure64_data_size;

	err = pure64_stream_set_pos(&util->disk_file.base, kernel_offset);
	if (err != 0) {
		kernel_done(&kernel_buf);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, kernel_buf.data, kernel_buf.size);
	if (err != 0) {
		kernel_done(&kernel_buf);
		return err;
	}

	kernel_done(&kernel_buf);

	return 0;
}

static int write_stage_two_bin(struct pure64_util *util) {

	unsigned long int stage_two_offset = 0;
	stage_two_offset += pure64_bootsector_size(util->config.bootsector);

	int err = pure64_stream_set_pos(&util->disk_file.base, stage_two_offset);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, pure64_data, pure64_data_size);
	if (err != 0)
		return err;

	return 0;
}

static int write_flat_partition(struct pure64_util *util) {

	int err = write_stage_two_bin(util);
	if (err != 0)
		return err;

	if (util->config.stage_three == PURE64_STAGE_THREE_KERNEL) {
		err = write_kernel_bin(util);
		if (err != 0)
			return err;
	} else {
		/* TODO : file system loader with flat partition */
		return PURE64_ENOSYS;
	}

	return 0;
}

static int write_fs_gpt(struct pure64_util *util,
                        struct pure64_gpt *gpt) {

	if (util->config.stage_three != PURE64_STAGE_THREE_LOADER)
		return 0;

	int err = pure64_gpt_set_entry_type(gpt, 2, PURE64_UUID_FILE_SYSTEM);
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_name(gpt, 2, u"Pure64 File System");
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_size(gpt, 2, util->config.fs_size);
	if (err != 0)
		return err;
	
	return 0;
}

static int write_kernel_gpt(struct pure64_util *util,
                            struct pure64_gpt *gpt) {

	int err = pure64_gpt_set_entry_type(gpt, 1, PURE64_UUID_STAGE_THREE);
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_name(gpt, 1, u"Pure64 Stage Three");
	if (err != 0)
		return err;

	struct kernel_buf kernel;

	err = kernel_open(&kernel, util->config.kernel);
	if (err != 0) {
		return err;
	}

	err = pure64_gpt_set_entry_size(gpt, 1, kernel.size);
	if (err != 0) {
		kernel_done(&kernel);
		return err;
	}

	err = pure64_stream_set_pos(&util->disk_file.base, gpt->primary_entries[1].first_lba * 512);
	if (err != 0) {
		kernel_done(&kernel);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, kernel.data, kernel.size);
	if (err != 0) {
		kernel_done(&kernel);
		return err;
	}

	kernel_done(&kernel);

	return 0;
}

static int write_loader_gpt(struct pure64_util *util,
                            struct pure64_gpt *gpt) {

	int err = pure64_gpt_set_entry_type(gpt, 1, PURE64_UUID_STAGE_THREE);
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_name(gpt, 1, u"Pure64 Stage Three");
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_size(gpt, 1, stage_three_data_size);
	if (err != 0)
		return err;

	err = pure64_stream_set_pos(&util->disk_file.base, gpt->primary_entries[1].first_lba * 512);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, stage_three_data, stage_three_data_size);
	if (err != 0)
		return err;

	return 0;
}

static int write_stage_three_gpt(struct pure64_util *util,
                                 struct pure64_gpt *gpt) {

	if (util->config.stage_three == PURE64_STAGE_THREE_LOADER)
		return write_loader_gpt(util, gpt);
	else if (util->config.stage_three == PURE64_STAGE_THREE_KERNEL)
		return write_kernel_gpt(util, gpt);
	else
		return PURE64_EINVAL;
}

static int write_stage_two_gpt(struct pure64_util *util,
                               struct pure64_gpt *gpt) {

	int err = pure64_gpt_set_entry_type(gpt, 0, PURE64_UUID_STAGE_TWO);
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_name(gpt, 0, u"Pure64 Stage Two");
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_size(gpt, 0, pure64_data_size);
	if (err != 0)
		return err;

	err = pure64_stream_set_pos(&util->disk_file.base, gpt->primary_entries[0].first_lba * 512);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, pure64_data, pure64_data_size);
	if (err != 0)
		return err;

	return 0;
}

static int write_gpt_partitions(struct pure64_util *util) {

	struct pure64_gpt gpt;

	pure64_gpt_init(&gpt);

	int err = pure64_gpt_format(&gpt, util->config.disk_size);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	err = write_stage_two_gpt(util, &gpt);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	err = write_stage_three_gpt(util, &gpt);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	err = write_fs_gpt(util, &gpt);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	err = pure64_gpt_export(&gpt, &util->disk_file.base);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	pure64_gpt_done(&gpt);

	return 0;
}

static int write_partitions(struct pure64_util *util) {

	if (util->config.partition_scheme == PURE64_PARTITION_SCHEME_NONE) {
		return write_flat_partition(util);
	} else if (util->config.partition_scheme == PURE64_PARTITION_SCHEME_GPT) {
		return write_gpt_partitions(util);
	}

	return PURE64_EINVAL;
}

void pure64_util_init(struct pure64_util *util) {
	pure64_config_init(&util->config);
	pure64_fstream_init(&util->disk_file);
	pure64_fs_init(&util->fs);
	util->errlog = stderr;
}

void pure64_util_done(struct pure64_util *util) {
	pure64_config_done(&util->config);
	pure64_fstream_done(&util->disk_file);
	pure64_fs_free(&util->fs);
}

int pure64_util_create_disk(struct pure64_util *util,
                            const char *path) {

	int err = pure64_fstream_open(&util->disk_file, path, "wb+");
	if (err != 0)
		return err;

	err = pure64_fstream_zero(&util->disk_file, util->config.disk_size);
	if (err != 0)
		return err;

	err = write_bootsector(util);
	if (err != 0)
		return err;

	err = write_partitions(util);
	if (err != 0)
		return err;

	return 0;
}

int pure64_util_open_config(struct pure64_util *util,
                            const char *path) {

	struct pure64_config_error error;

	int err = pure64_config_load(&util->config, path, &error);
	if (err != 0) {
		if (error.line > 0)
			fprintf(util->errlog, "%s:%lu: %s\n", path, error.line, error.desc);
		else
			fprintf(util->errlog, "%s: %s\n", path, error.desc);
		return err;
	}

	return 0;
}

int pure64_util_mkgpt(struct pure64_util *util,
                      const struct pure64_uuid *disk_uuid) {

	uint64_t disk_size = 0;

	int err = pure64_stream_get_size(&util->disk_file.base, &disk_size);
	if (err != 0)
		return err;

	struct pure64_gpt gpt;

	pure64_gpt_init(&gpt);

	err = pure64_gpt_format(&gpt, disk_size);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	pure64_gpt_set_disk_uuid(&gpt, disk_uuid);

	err = pure64_gpt_export(&gpt, &util->disk_file.base);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	pure64_gpt_done(&gpt);

	return 0;
}

int pure64_util_open_disk(struct pure64_util *util,
                            const char *path) {

	int err = pure64_fstream_open(&util->disk_file, path, "rb+");
	if (err != 0)
		return err;

	return 0;
}

int pure64_util_set_bootsector(struct pure64_util *util,
                               const void *bootsector_data,
                               uint64_t bootsector_size) {

	int err = pure64_stream_set_pos(&util->disk_file.base, 0x00);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, bootsector_data, bootsector_size);
	if (err != 0)
		return err;

	if (bootsector_size < 512) {

		size_t buf_size = 512 - bootsector_size;

		void *buf = calloc(1, buf_size);
		if (buf == NULL)
			return PURE64_ENOMEM;

		err = pure64_stream_write(&util->disk_file.base, buf, buf_size);
		if (err != 0) {
			free(buf);
			return err;
		}

		free(buf);
	}

	return 0;
}

static int set_stage_two_bin(struct pure64_util *util,
                             const void *stage_two_data,
                             uint64_t stage_two_size) {

	int err = pure64_stream_set_pos(&util->disk_file.base, 512);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, stage_two_data, stage_two_size);
	if (err != 0)
		return err;

	return 0;
}

static int set_stage_two_gpt(struct pure64_util *util,
                             struct pure64_gpt *gpt,
                             const void *stage_two_data,
                             uint64_t stage_two_size) {

	uint32_t entry_index = 0;

	int err = pure64_gpt_find_unused_entry(gpt, &entry_index);
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_type(gpt, entry_index, PURE64_UUID_STAGE_TWO);
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_name(gpt, entry_index, u"Pure64 Stage Two");
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_size(gpt, entry_index, stage_two_size);
	if (err != 0)
		return err;

	uint64_t offset = 0;

	err = pure64_gpt_get_partition_offset(gpt, entry_index, &offset);
	if (err != 0)
		return err;

	err = pure64_stream_set_pos(&util->disk_file.base, offset);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, stage_two_data, stage_two_size);
	if (err != 0)
		return err;

	err = pure64_gpt_export(gpt, &util->disk_file.base);
	if (err != 0)
		return err;

	return 0;
}

int pure64_util_set_stage_two(struct pure64_util *util,
                              const void *stage_two_data,
                              uint64_t stage_two_size) {

	struct pure64_gpt gpt;

	pure64_gpt_init(&gpt);

	int err = pure64_gpt_import(&gpt, &util->disk_file.base);
	if (err != 0) {
		err = set_stage_two_bin(util, stage_two_data, stage_two_size);
	} else {
		err = set_stage_two_gpt(util, &gpt, stage_two_data, stage_two_size);
	}

	pure64_gpt_done(&gpt);

	return err;
}
