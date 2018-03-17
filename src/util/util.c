/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "util.h"

#include "config.h"

#include "pure64-data.h"

#include <pure64/error.h>
#include <pure64/gpt.h>

#include <stdlib.h>

static int write_bootsector(struct pure64_util *util,
                            const struct pure64_config *config) {

	const void *bootsector = pure64_bootsector_data(config->bootsector);

	unsigned long int bootsector_size = pure64_bootsector_size(config->bootsector);

	int err = pure64_stream_set_pos(&util->disk_file.base, 0);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, bootsector, bootsector_size);
	if (err != 0)
		return err;

	return 0;
}

static int write_stage_two_bin(struct pure64_util *util,
                               const struct pure64_config *config) {

	unsigned long int stage_two_offset = 0;
	stage_two_offset += pure64_bootsector_size(config->bootsector);

	int err = pure64_stream_set_pos(&util->disk_file.base, stage_two_offset);
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, pure64_data, pure64_data_size);
	if (err != 0)
		return err;

	return 0;
}

static int write_stage_two_gpt(struct pure64_util *util,
                               const struct pure64_config *config) {
	(void) util;
	(void) config;
	return 0;
}

static int write_stage_two(struct pure64_util *util,
                           const struct pure64_config *config) {

	if (config->partition_scheme == PURE64_PARTITION_SCHEME_NONE) {
		return write_stage_two_bin(util, config);
	} else if (config->partition_scheme == PURE64_PARTITION_SCHEME_GPT) {
		return write_stage_two_gpt(util, config);
	}

	return PURE64_EINVAL;
}

static int write_kernel(struct pure64_util *util,
                        const struct pure64_config *config) {

	const char *path = config->kernel;
	if (path == NULL)
		path = "kernel";

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

	unsigned long int kernel_offset = 0;
	kernel_offset += pure64_bootsector_size(config->bootsector);
	kernel_offset += pure64_data_size;

	int err = pure64_stream_set_pos(&util->disk_file.base, kernel_offset);
	if (err != 0) {
		free(buf);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, buf, kernel_size);
	if (err != 0) {
		free(buf);
		return err;
	}

	free(buf);

	return 0;
}

void pure64_util_init(struct pure64_util *util) {
	pure64_fstream_init(&util->disk_file);
	pure64_fs_init(&util->fs);
	util->errlog = stderr;
}

void pure64_util_done(struct pure64_util *util) {
	pure64_fstream_done(&util->disk_file);
	pure64_fs_free(&util->fs);
}

int pure64_util_create_disk(struct pure64_util *util,
                            const char *config_path,
                            const char *path) {

	struct pure64_config config;
	struct pure64_config_error error;

	pure64_config_init(&config);

	int err = pure64_config_load(&config, config_path, &error);
	if (err != 0) {

		if (error.line > 0)
			fprintf(util->errlog, "%s:%lu: %s\n", config_path, error.line, error.desc);
		else
			fprintf(util->errlog, "%s: %s\n", config_path, error.desc);

		pure64_config_done(&config);

		return err;
	}

	err = pure64_fstream_open(&util->disk_file, path, "wb+");
	if (err != 0) {
		pure64_config_done(&config);
		return err;
	}

	err = pure64_fstream_zero(&util->disk_file, config.disk_size);
	if (err != 0) {
		pure64_config_done(&config);
		return err;
	}

	err = write_bootsector(util, &config);
	if (err != 0) {
		pure64_config_done(&config);
		return err;
	}

	err = write_stage_two(util, &config);
	if (err != 0) {
		pure64_config_done(&config);
		return err;
	}

	if (config.stage_three == PURE64_STAGE_THREE_KERNEL) {
		err = write_kernel(util, &config);
		if (err != 0) {
			pure64_config_done(&config);
			return err;
		}
	}

	pure64_config_done(&config);

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
