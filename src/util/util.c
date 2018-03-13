/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "util.h"

#include <pure64/error.h>
#include <pure64/gpt.h>

#include <stdlib.h>

void pure64_util_init(struct pure64_util *util) {
	pure64_fstream_init(&util->disk_file);
	pure64_fs_init(&util->fs);
}

void pure64_util_done(struct pure64_util *util) {
	pure64_fstream_done(&util->disk_file);
	pure64_fs_free(&util->fs);
}

int pure64_util_create_disk(struct pure64_util *util,
                            const char *path,
                            uint64_t disk_size) {

	int err = pure64_fstream_open(&util->disk_file, path, "wb+");
	if (err != 0)
		return err;

	disk_size = ((disk_size + 511) / 512) * 512;

	err = pure64_fstream_zero(&util->disk_file, disk_size);
	if (err != 0)
		return err;

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
