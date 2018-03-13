#ifndef PURE64_UTIL_H
#define PURE64_UTIL_H

#include <pure64/fs.h>

#include "fstream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_uuid;

struct pure64_util {
	/** The file associated with the
	 * Pure64 disk image. */
	struct pure64_fstream disk_file;
	/** The file system contained on
	 * the disk image. */
	struct pure64_fs fs;
};

/** Initializes the utility.
 * @param util An uninitialized utilty structure.
 * */

void pure64_util_init(struct pure64_util *util);

/** Releases resources allocated by
 * the utility structure.
 * @param util An initialized utility structure.
 * */

void pure64_util_done(struct pure64_util *util);

/** Creates a new disk image.
 * @param util An initialized utility structure.
 * @param path The path to create the disk image at.
 * @param disk_size The size of the disk, in bytes.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_create_disk(struct pure64_util *util,
                            const char *path,
                            uint64_t disk_size);

/** Opens an existing disk image.
 * @param util An initialized utility structure.
 * @param path The path of the disk image.
 * @returns Zero on success, an error code on failure.
 */

int pure64_util_open_disk(struct pure64_util *util,
                          const char *path);

/** Creates a partition table in GPT format.
 * @param util An initialized utility structure.
 * @param disk_uuid The UUID to use for the disk.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_mkgpt(struct pure64_util *util,
                      const struct pure64_uuid *disk_uuid);

/** Sets the bootsector of the disk image.
 * @param util An initialized utility structure.
 * @param bootsector_data The boot sector data
 * @param bootsector_size The number of bytes occupied
 * by the bootsector. This should be equal to or
 * less than 512 bytes.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_set_bootsector(struct pure64_util *util,
                               const void *bootsector_data,
                               uint64_t bootsector_size);

/** Sets the second stage loader onto the image.
 * If the disk is GPT formatted, then stage two
 * is added to a new partition. If it is not GPT
 * formatted, then stage two falls right after the
 * boot sector.
 * @param util An initialized utility structure.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_set_stage_two(struct pure64_util *util,
                              const void *stage_two_data,
                              uint64_t stage_two_size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_UTIL_H */
