#ifndef PURE64_UTIL_H
#define PURE64_UTIL_H

#include <pure64/fs.h>

#include <pure64/lang/config.h>

#include "fstream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_uuid;

struct pure64_util {
	struct pure64_config config;
	/** The file associated with the
	 * Pure64 disk image. */
	struct pure64_fstream disk_file;
	/** The file system contained on
	 * the disk image. */
	struct pure64_fs fs;
	/** The standard error output of
	 * the utility. */
	FILE *errlog;
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
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_create_disk(struct pure64_util *util,
                            const char *path);

/** Opens an existing disk image.
 * @param util An initialized utility structure.
 * @param path The path of the disk image.
 * @returns Zero on success, an error code on failure.
 */

int pure64_util_open_disk(struct pure64_util *util,
                          const char *path);

/** Opens a configuration file.
 * @param util An initialized utility structure.
 * @param path The path of the config file.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_open_config(struct pure64_util *util,
                            const char *path);

/** Saves information in memory onto the disk.
 * This should be called before @ref pure64_util_done,
 * if the changes made to the disk should remain.
 * This does not have to be called after creating
 * a disk.
 * @param util An initialized utility structure.
 * @returns Zero on success, an error code on failure.
 * */

int pure64_util_save_disk(struct pure64_util *util);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_UTIL_H */
