#ifndef PURE64_CONFIG_H
#define PURE64_CONFIG_H

#include <pure64/core/arch.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Describes which bootsector
 * that is contained by a Pure64 image.
 * @ingroup lang-api
 * */

enum pure64_bootsector {
	/** This means that there is no
	 * bootsector on the disk. */
	PURE64_BOOTSECTOR_NONE,
	/** This means that the bootsector
	 * is MBR formatted. */
	PURE64_BOOTSECTOR_MBR,
	/** This means that the bootsector
	 * is made for PXE booting. */
	PURE64_BOOTSECTOR_PXE,
	/** This means that the bootsector
	 * is made to be loaded as a multiboot
	 * module. */
	PURE64_BOOTSECTOR_MULTIBOOT,
	/** This means that the bootsector
	 * is made to be loaded as a multiboot2
	 * module. */
	PURE64_BOOTSECTOR_MULTIBOOT2
};

/** Gets the size of a specified bootsector.
 * @param bootsector The bootsector to get the size of.
 * @returns The size, in bytes, of the specified bootsector.
 * @ingroup lang-api
 * */

unsigned long int pure64_bootsector_size(enum pure64_bootsector bootsector);

/** Describes a partitioning
 * scheme supported by Pure64.
 * @ingroup lang-api
 * */

enum pure64_partition_scheme {
	/** This means there is not
	 * a partitioning scheme. */
	PURE64_PARTITION_SCHEME_NONE,
	/** This means the partitioning
	 * scheme is GPT. */
	PURE64_PARTITION_SCHEME_GPT
};

/** Indicates what happens after the
 * second stage in the boot process.
 * @ingroup lang-api
 * */

enum pure64_stage_three {
	/** This means that stage three has
	 * not been specified. */
	PURE64_STAGE_THREE_NONE,
	/** This means that a kernel is loaded
	 * directory after stage two. */
	PURE64_STAGE_THREE_KERNEL,
	/** This means that the file system loader
	 * is started after stage two. */
	PURE64_STAGE_THREE_LOADER
};

/** This is a structure that describes
 * a syntax error that occured in a config file.
 * @ingroup lang-api
 * */

struct pure64_config_error {
	/** The description of the error. */
	const char *desc;
	/** The line that the error occured on. */
	unsigned long int line;
	/** The column that the error occured on. */
	unsigned long int column;
};

/** Represents a disk configuration file.
 * The configuration is used for detecting
 * invalid settings and determining offsets
 * of various code segments and data.
 * @ingroup lang-api
 * */

struct pure64_config {
	/** The architecture of the image. */
	enum pure64_arch arch;
	/** The bootsector used for the disk image. */
	enum pure64_bootsector bootsector;
	/** The partitioning scheme used by the disk image. */
	enum pure64_partition_scheme partition_scheme;
	/** Indicates what happens after the second stage
	 * boot loader. */
	enum pure64_stage_three stage_three;
	/** The size, in bytes, of the disk. */
	unsigned long int disk_size;
	/** The size, in bytes, to reserve for the file system. */
	unsigned long int fs_size;
	/** The path of the kernel to load. This option
	 * is only valid if the stage three loader is
	 * specified to load a kernel. */
	char *kernel;
	/** The root path to Pure64's bootsectors,
	 * initialization code, and more. */
	char *resource_path;
};

/** Initializes a configuration file with default values.
 * @param config The configuration structure to initialize.
 * @ingroup lang-api
 * */

void pure64_config_init(struct pure64_config *config);

/** Releases memory allocated by a configuration.
 * @param config An initialized config structure.
 * @ingroup lang-api
 * */

void pure64_config_done(struct pure64_config *config);

/** Parses a configuration file.
 * @param config An initialized configuration structure.
 * @param source A null-terminated configuration file.
 * @param error An error structure that will describe
 * a syntax error if one occurs.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_config_parse(struct pure64_config *config,
                        const char *source,
                        struct pure64_config_error *error);

/** Loads a configuration from a file.
 * @param config An initialized config structure.
 * @param filename The path of the configuration file.
 * @param error A pointer to an error structure that
 * will be describe an error if one occurs.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_config_load(struct pure64_config *config,
                       const char *filename,
                       struct pure64_config_error *error);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_CONFIG_H */
