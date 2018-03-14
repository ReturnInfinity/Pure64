#ifndef PURE64_CONFIG_H
#define PURE64_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/** Describes which bootsector
 * that is contained by a Pure64 image.
 * */

enum pure64_bootsector {
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

/** Describes a partitioning
 * scheme supported by Pure64.
 * */

enum pure64_partition_scheme {
	/** This means there is not
	 * a partitioning scheme. */
	PURE64_PARTITION_SCHEME_NONE,
	/** This means the partitioning
	 * scheme is GPT. */
	PURE64_PARTITION_SCHEME_GPT
};

/** Represents a disk configuration file.
 * The configuration is used for detecting
 * invalid settings and determining offsets
 * of various code segments and data.
 * */

struct pure64_config {
	/** The bootsector used for the disk image. */
	enum pure64_bootsector bootsector;
	/** The partitioning scheme used by the disk image. */
	enum pure64_partition_scheme partition_scheme;
};

void pure64_config_init(struct pure64_config *config);

/** Parses a configuration file.
 * @param config An initialized configuration structure.
 * @param source A null-terminated configuration file.
 * */

int pure64_config_parse(struct pure64_config *config,
                        const char *source);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_CONFIG_H */
