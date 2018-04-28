/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "util.h"

#include <pure64/lang/config.h>

#include <pure64/error.h>
#include <pure64/gpt.h>
#include <pure64/mbr.h>
#include <pure64/partition.h>

#include <stdlib.h>
#include <string.h>

#ifndef PURE64_INSTALL_PATH
#define PURE64_INSTALL_PATH "/opt/return-infinity"
#endif

#ifndef PURE64_RESOURCE_PATH
#define PURE64_RESOURCE_PATH PURE64_INSTALL_PATH "/share/pure64/resources"
#endif

#ifndef PURE64_SIZE
#define PURE64_SIZE 4096
#endif

const unsigned long int pure64_data_size = PURE64_SIZE;

struct file_buf {
	void *data;
	unsigned long int size;
};

static int file_open(struct file_buf *file_buf,
                       const char *path) {

	file_buf->data = NULL;
	file_buf->size = 0;

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

	file_buf->data = buf;
	file_buf->size = (unsigned long int) kernel_size;

	return 0;
}

static void file_done(struct file_buf *file_buf) {
	free(file_buf->data);
	file_buf->data = NULL;
	file_buf->size = 0;
}

static const char *get_root_resource_path(const struct pure64_config *config) {

	if (config->resource_path == NULL) {

		const char *respath = getenv("PURE64_RESOURCE_PATH");
		if (respath != NULL)
			return respath;

		return PURE64_RESOURCE_PATH;

	} else {
		return config->resource_path;
	}
}

char *get_full_resource_path(const struct pure64_config *config,
                             const char *suffix_path) {

	const char *resource_prefix = get_root_resource_path(config);
	if (resource_prefix == NULL)
		return NULL;

	unsigned long int path_size = 0;
	path_size += strlen(resource_prefix);
	path_size += 1; /* for '/' */
	path_size += strlen(suffix_path);

	char *full_resource_path = malloc(path_size + 1);
	if (full_resource_path == NULL)
		return NULL;

	strcpy(full_resource_path, resource_prefix);
	strcat(full_resource_path, "/");
	strcat(full_resource_path, suffix_path);

	return full_resource_path;
}

static int resource_get_size(const struct pure64_config *config,
                             pure64_uint64 *size,
                             const char *suffix_path) {

	char *full_path = get_full_resource_path(config, suffix_path);
	if (full_path == NULL)
		return PURE64_ENOMEM;

	FILE *file = fopen(full_path, "rb");
	if (file == NULL) {
		free(full_path);
		return PURE64_ENOENT;
	}

	free(full_path);

	int err = fseek(file, 0, SEEK_END);
	if (err != 0) {
		fclose(file);
		return PURE64_EINVAL;
	}

	long int file_size = ftell(file);
	if (file_size == -1L) {
		fclose(file);
		return PURE64_EINVAL;
	}

	fclose(file);

	if (size != NULL)
		*size = (pure64_uint64) file_size;

	return 0;
}

static int resource_open(const struct pure64_config *config,
                         struct file_buf *file_buf,
                         const char *suffix_path) {

	char *full_path = get_full_resource_path(config, suffix_path);
	if (full_path == NULL)
		return PURE64_ENOMEM;

	int err = file_open(file_buf, full_path);
	if (err != 0) {
		free(full_path);
		return err;
	}

	free(full_path);

	return 0;
}

static int bootsector_open(const struct pure64_config *config,
                           struct file_buf *file_buf,
                           enum pure64_bootsector bootsector) {

	switch (bootsector) {
	case PURE64_BOOTSECTOR_MBR:
		return resource_open(config, file_buf, "x86_64/bootsectors/mbr.sys");
	case PURE64_BOOTSECTOR_PXE:
		return resource_open(config, file_buf, "x86_64/bootsectors/pxe.sys");
	case PURE64_BOOTSECTOR_MULTIBOOT:
		return resource_open(config, file_buf, "x86_64/bootsectors/mulitboot.sys");
	case PURE64_BOOTSECTOR_MULTIBOOT2:
		return resource_open(config, file_buf, "x86_64/bootsectors/mulitboot2.sys");
	default:
		break;
	}

	return PURE64_EINVAL;
}

static int write_bootsector(struct pure64_util *util) {

	int err = pure64_stream_set_pos(&util->disk_file.base, 0);
	if (err != 0)
		return err;

	struct file_buf file_buf;

	err = bootsector_open(&util->config, &file_buf, util->config.bootsector);
	if (err != 0) {
		fprintf(util->errlog, "Failed to open bootsector: %s\n", pure64_strerror(err));
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, file_buf.data, file_buf.size);
	if (err != 0) {
		fprintf(util->errlog, "Failed to write bootsector: %s\n", pure64_strerror(err));
		file_done(&file_buf);
		return err;
	}

	file_done(&file_buf);

	return 0;
}

static int write_kernel_bin(struct pure64_util *util) {

	const char *path = util->config.kernel;
	if (path == NULL)
		path = "kernel";

	struct file_buf file_buf;

	int err = file_open(&file_buf, path);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	unsigned long int kernel_offset = 0;
	kernel_offset += pure64_bootsector_size(util->config.bootsector);
	kernel_offset += pure64_data_size;

	err = pure64_stream_set_pos(&util->disk_file.base, kernel_offset);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, file_buf.data, file_buf.size);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	file_done(&file_buf);

	return 0;
}

static int write_stage_two_bin(struct pure64_util *util) {

	unsigned long int stage_two_offset = 0;
	stage_two_offset += pure64_bootsector_size(util->config.bootsector);

	int err = pure64_stream_set_pos(&util->disk_file.base, stage_two_offset);
	if (err != 0)
		return err;

	struct file_buf file_buf;

	err = resource_open(&util->config, &file_buf, "x86_64/pure64.sys");
	if (err != 0)
		return err;

	err = pure64_stream_write(&util->disk_file.base, file_buf.data, file_buf.size);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	file_done(&file_buf);

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

	struct pure64_partition partition;

	pure64_partition_init(&partition);

	pure64_partition_set_offset(&partition, gpt->primary_entries[2].first_lba * 512);

	pure64_partition_set_size(&partition, util->config.fs_size);

	pure64_partition_set_disk(&partition, &util->disk_file.base);

	err = pure64_fs_make_dir(&util->fs, "/boot");
	if (err != 0)
		return err;

	err = pure64_fs_export(&util->fs, &partition.stream);
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

	struct file_buf kernel;

	err = file_open(&kernel, util->config.kernel);
	if (err != 0) {
		return err;
	}

	err = pure64_gpt_set_entry_size(gpt, 1, kernel.size);
	if (err != 0) {
		file_done(&kernel);
		return err;
	}

	err = pure64_stream_set_pos(&util->disk_file.base, gpt->primary_entries[1].first_lba * 512);
	if (err != 0) {
		file_done(&kernel);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, kernel.data, kernel.size);
	if (err != 0) {
		file_done(&kernel);
		return err;
	}

	file_done(&kernel);

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

	struct file_buf file_buf;

	err = resource_open(&util->config, &file_buf, "x86_64/stage-three.sys");
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_size(gpt, 1, file_buf.size);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	err = pure64_stream_set_pos(&util->disk_file.base, gpt->primary_entries[1].first_lba * 512);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, file_buf.data, file_buf.size);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	file_done(&file_buf);

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

	struct file_buf file_buf;

	err = resource_open(&util->config, &file_buf, "x86_64/pure64.sys");
	if (err != 0)
		return err;

	err = pure64_gpt_set_entry_size(gpt, 0, pure64_data_size);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	err = pure64_stream_set_pos(&util->disk_file.base, gpt->primary_entries[0].first_lba * 512);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	err = pure64_stream_write(&util->disk_file.base, file_buf.data, file_buf.size);
	if (err != 0) {
		file_done(&file_buf);
		return err;
	}

	file_done(&file_buf);

	return 0;
}

static int update_mbr_gpt(struct pure64_util *util,
                          struct pure64_gpt *gpt) {

	pure64_uint64 pure64_data_size = 0;

	pure64_uint64 stage_three_data_size = 0;

	int err = 0;
	err |= resource_get_size(&util->config, &pure64_data_size, "x86_64/pure64.sys");
	err |= resource_get_size(&util->config, &stage_three_data_size, "x86_64/stage-three.sys");
	if (err != 0)
		return err;

	struct pure64_mbr mbr;

	pure64_mbr_zero(&mbr);

	err = pure64_mbr_read(&mbr, &util->disk_file.base);
	if (err != 0)
		return err;

	mbr.st2dap.sector = gpt->primary_entries[0].first_lba;
	mbr.st2dap.sector_count = (pure64_data_size + 511) / 512;

	mbr.st3dap.sector = gpt->primary_entries[1].first_lba;
	mbr.st3dap.sector_count = (stage_three_data_size + 511) / 512;

	err = pure64_mbr_write(&mbr, &util->disk_file.base);
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

	err = update_mbr_gpt(util, &gpt);
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

static int import_fs_gpt(struct pure64_util *util,
                         struct pure64_gpt *gpt) {

	if (util->config.stage_three != PURE64_STAGE_THREE_LOADER)
		return 0;

	struct pure64_partition partition;

	pure64_partition_init(&partition);

	pure64_partition_set_offset(&partition, gpt->primary_entries[2].first_lba * 512);

	pure64_partition_set_size(&partition, util->config.fs_size);

	pure64_partition_set_disk(&partition, &util->disk_file.base);

	int err = pure64_fs_import(&util->fs, &partition.stream);
	if (err != 0)
		return err;

	return 0;
}

static int import_gpt(struct pure64_util *util) {

	struct pure64_gpt gpt;

	pure64_gpt_init(&gpt);

	int err = pure64_gpt_import(&gpt, &util->disk_file.base);
	if (err != 0)
		return err;

	err = import_fs_gpt(util, &gpt);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	pure64_gpt_done(&gpt);

	return 0;
}

static int save_fs_gpt(struct pure64_util *util,
                       struct pure64_gpt *gpt) {

	struct pure64_partition partition;

	pure64_partition_init(&partition);

	pure64_partition_set_offset(&partition, gpt->primary_entries[2].first_lba * 512);

	pure64_partition_set_size(&partition, util->config.fs_size);

	pure64_partition_set_disk(&partition, &util->disk_file.base);

	int err = pure64_fs_export(&util->fs, &partition.stream);
	if (err != 0)
		return err;

	return 0;
}

static int save_gpt(struct pure64_util *util) {

	struct pure64_gpt gpt;

	pure64_gpt_init(&gpt);

	int err = pure64_gpt_import(&gpt, &util->disk_file.base);
	if (err != 0) {
		pure64_gpt_done(&gpt);
		return err;
	}

	err = save_fs_gpt(util, &gpt);
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
		if ((error.line > 0) && (error.column > 0))
			fprintf(util->errlog, "%s:%lu:%lu: %s\n", path, error.line, error.column, error.desc);
		else if (error.line > 0)
			fprintf(util->errlog, "%s:%lu: %s\n", path, error.line, error.desc);
		else
			fprintf(util->errlog, "%s: %s\n", path, error.desc);
		return err;
	}

	return 0;
}

int pure64_util_open_disk(struct pure64_util *util,
                            const char *path) {

	int err = pure64_fstream_open(&util->disk_file, path, "rb+");
	if (err != 0)
		return err;

	if (util->config.partition_scheme == PURE64_PARTITION_SCHEME_GPT) {
		err = import_gpt(util);
		if (err != 0)
			return err;
	}

	return 0;
}

int pure64_util_save_disk(struct pure64_util *util) {

	if (util->config.stage_three != PURE64_STAGE_THREE_LOADER)
		return 0;

	if (util->config.partition_scheme == PURE64_PARTITION_SCHEME_GPT)
		return save_gpt(util);

	return 0;
}
