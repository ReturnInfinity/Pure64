#include "config.h"

#include "mbr-data.h"
#include "multiboot-data.h"
#include "multiboot2-data.h"
#include "pxe-data.h"

#include <pure64/error.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char is_whitespace(char c) {
	if ((c == ' ')
	 || (c == '\t'))
		return 1;
	else
		return 0;
}

struct pure64_var {
	const char *key;
	unsigned int key_size;
	const char *value;
	unsigned int value_size;
};

static unsigned int skip_whitespace(const char *line,
                                    unsigned int line_pos,
                                    unsigned int line_size) {

	unsigned int i = line_pos;

	while (i < line_size) {

		if (!is_whitespace(line[i]))
			break;

		i++;
	}

	return i;
}

static int parse_var(struct pure64_var *var,
                     const char *line,
                     unsigned int line_size,
                     struct pure64_config_error *error) {

	unsigned int i = skip_whitespace(line, 0, line_size);

	/* get key size */

	var->key = &line[i];

	var->key_size = 0;

	unsigned char colon_found = 0;

	while (i < line_size) {

		char c = line[i];

		if (c == ':') {
			i++;
			colon_found = 1;
			break;
		} else if (is_whitespace(c)) {
			break;
		}

		i++;

		var->key_size++;
	}

	/* skip trailing whitespace */

	if (!colon_found) {
		i = skip_whitespace(line, i, line_size);
		if ((i >= line_size) || (line[i] != ':')) {
			error->desc = "Expected ':'";
			return PURE64_EINVAL;
		}
		i++;
	}

	if (i >= line_size) {
		error->desc = "Expected a value following variable key";
		return PURE64_EINVAL;
	}

	i = skip_whitespace(line, i, line_size);

	/* parse value */

	var->value = &line[i];

	var->value_size = 0;

	while (i < line_size) {

		char c = line[i];
		if (c == '\n')
			break;
		else if ((var->value_size > 0) && is_whitespace(c))
			break;

		i++;

		var->value_size++;
	}

	return 0;
}

static unsigned char is_blank(const char *line, unsigned int line_size) {
	if (skip_whitespace(line, 0, line_size) == line_size)
		return 1;
	else
		return 0;
}

static unsigned char is_comment(const char *line, unsigned int line_size) {

	unsigned int i = 0;

	i = skip_whitespace(line, 0, line_size);
	if (i == line_size)
		return 0;
	else if (line[i] == '#')
		return 1;
	else
		return 0;
}

static unsigned char is_key(const struct pure64_var *var, const char *key) {

	unsigned int key_size = strlen(key);

	if (var->key_size != key_size) {
		return 0;
	} else if (memcmp(var->key, key, key_size) != 0) {
		return 0;
	}

	return 1;
}

static unsigned char is_value(const struct pure64_var *var, const char *value) {

	unsigned int value_size = strlen(value);

	if (var->value_size != value_size) {
		return 0;
	} else if (memcmp(var->value, value, value_size) != 0) {
		return 0;
	}

	return 1;
}

static unsigned char file_exists(const char *path) {

	FILE *file = fopen(path, "rb");

	if (file == NULL)
		return 0;

	fclose(file);

	return 1;
}

static int parse_disk_size(struct pure64_config *config,
                           const char *str,
                           unsigned int str_size) {

	const unsigned long int TiB = 1024UL * 1024UL * 1024UL * 1024UL;
	const unsigned long int GiB = 1024UL * 1024UL * 1024UL;
	const unsigned long int MiB = 1024UL * 1024UL;
	const unsigned long int KiB = 1024UL;

	const unsigned long int TB = 1000UL * 1000UL * 1000UL * 1000UL;
	const unsigned long int GB = 1000UL * 1000UL * 1000UL;
	const unsigned long int MB = 1000UL * 1000UL;
	const unsigned long int KB = 1000UL;

	if (str_size == 0)
		return -1;

	char *tmp = malloc(str_size + 1);
	if (tmp == NULL)
		return -1;

	memcpy(tmp, str, str_size);

	tmp[str_size] = 0;

	unsigned long int multiplier = 1;

	const char *suffix = &tmp[str_size - 1];
	if (strcmp(suffix, "T") == 0) {
		multiplier = TiB;
	} else if (strcmp(suffix, "G") == 0) {
		multiplier = GiB;
	} else if (strcmp(suffix, "M") == 0) {
		multiplier = MiB;
	} else if (strcmp(suffix, "K") == 0) {
		multiplier = KiB;
	}

	if (multiplier != 1)
		str_size--;

	if ((multiplier == 1) && (str_size > 2)) {
		suffix = &tmp[str_size - 3];
		if (strcmp(suffix, "TiB") == 0) {
			multiplier = TiB;
		} else if (strcmp(suffix, "GiB") == 0) {
			multiplier = GiB;
		} else if (strcmp(suffix, "MiB") == 0) {
			multiplier = MiB;
		} else if (strcmp(suffix, "KiB") == 0) {
			multiplier = KiB;
		}
		if (multiplier != 1)
			str_size -= 3;
	}

	if ((multiplier == 1) && (str_size > 1)) {
		suffix = &tmp[str_size - 2];
		if (strcmp(suffix, "TB") == 0) {
			multiplier = TB;
		} else if (strcmp(suffix, "GB") == 0) {
			multiplier = GB;
		} else if (strcmp(suffix, "MB") == 0) {
			multiplier = MB;
		} else if (strcmp(suffix, "KB") == 0) {
			multiplier = KB;
		}
		if (multiplier != 1)
			str_size -= 2;
	}

	tmp[str_size] = 0;

	if (sscanf(tmp, "%lu", &config->disk_size) != 1) {
		free(tmp);
		return -1;
	}

	config->disk_size *= multiplier;

	free(tmp);

	return 0;
}

static int handle_var(struct pure64_config *config,
                      const struct pure64_var *var,
                      struct pure64_config_error *error) {

	if (is_key(var, "bootsector")) {
		if (is_value(var, "mbr")) {
			config->bootsector = PURE64_BOOTSECTOR_MBR;
		} else if (is_value(var, "pxe")) {
			config->bootsector = PURE64_BOOTSECTOR_PXE;
		} else if (is_value(var, "multiboot")) {
			config->bootsector = PURE64_BOOTSECTOR_MULTIBOOT;
		} else if (is_value(var, "multiboot2")) {
			config->bootsector = PURE64_BOOTSECTOR_MULTIBOOT2;
		} else {
			error->desc = "Unknown bootsector type";
			return PURE64_EINVAL;
		}
	} else if (is_key(var, "kernel")) {
		free(config->kernel);
		config->kernel = malloc(var->value_size + 1);
		if (config->kernel == NULL) {
			error->desc = "Failed to allocate memory for kernel path";
			return PURE64_ENOMEM;
		}
		memcpy(config->kernel, var->value, var->value_size);
		config->kernel[var->value_size] = 0;
		if (!file_exists(config->kernel)) {
			error->desc = "Kernel does not exist";
			return PURE64_ENOMEM;
		}
	} else if (is_key(var, "stage-three")) {
		if (is_value(var, "kernel")) {
			config->stage_three = PURE64_STAGE_THREE_KERNEL;
		} else if (is_value(var, "loader")) {
			config->stage_three = PURE64_STAGE_THREE_LOADER;
		} else {
			error->desc = "Unknown stage three type";
			return PURE64_EINVAL;
		}
	} else if (is_key(var, "partition-scheme")) {
		if (is_value(var, "none")) {
			config->partition_scheme = PURE64_PARTITION_SCHEME_NONE;
		} else if (is_value(var, "gpt")) {
			config->partition_scheme = PURE64_PARTITION_SCHEME_GPT;
		} else {
			error->desc = "Unknown partition scheme";
			return PURE64_EINVAL;
		}
	} else if (is_key(var, "disk-size")) {
		if (var->value_size == 0) {
			error->desc = "Size not specified";
			return -1;
		}
		if (parse_disk_size(config, var->value, var->value_size) != 0) {
			error->desc = "Invalid size";
			return PURE64_EINVAL;
		}
	} else if (is_key(var, "arch")) {
		if (is_value(var, "x86_64")) {
			config->arch = PURE64_ARCH_x86_64;
		} else {
			error->desc = "Unsupported architecture";
			return PURE64_EINVAL;
		}
	} else {
		error->desc = "Unknown variable key";
		return PURE64_EINVAL;
	}

	return 0;
}

static int validate_vars(const struct pure64_config *config,
                         struct pure64_config_error *error) {

	if (config->arch == PURE64_ARCH_NONE) {
		error->desc = "Architecture not specified";
		return PURE64_EINVAL;
	}

	if (config->stage_three == PURE64_STAGE_THREE_NONE) {
		error->desc = "Stage three not specified";
		return PURE64_EINVAL;
	}

	if ((config->stage_three == PURE64_STAGE_THREE_LOADER)
	 && (config->bootsector != PURE64_BOOTSECTOR_MBR)) {
		error->desc = "File system loader only valid with MBR bootsector";
		return PURE64_EINVAL;
	}

	if ((config->stage_three == PURE64_STAGE_THREE_KERNEL)
	 && (config->kernel == NULL)) {
		error->desc = "Kernel not specifed";
		return PURE64_EINVAL;
	}

	return 0;
}

const void *pure64_bootsector_data(enum pure64_bootsector bootsector) {

	switch (bootsector) {
	case PURE64_BOOTSECTOR_MBR:
		return mbr_data;
	case PURE64_BOOTSECTOR_MULTIBOOT:
		return multiboot_data;
	case PURE64_BOOTSECTOR_MULTIBOOT2:
		return multiboot2_data;
	case PURE64_BOOTSECTOR_PXE:
		return pxe_data;
	default:
		break;
	}

	return NULL;
}

unsigned long int pure64_bootsector_size(enum pure64_bootsector bootsector) {

	switch (bootsector) {
	case PURE64_BOOTSECTOR_MBR:
		return mbr_data_size;
	case PURE64_BOOTSECTOR_MULTIBOOT:
		return multiboot_data_size;
	case PURE64_BOOTSECTOR_MULTIBOOT2:
		return multiboot2_data_size;
	case PURE64_BOOTSECTOR_PXE:
		return pxe_data_size;
	default:
		break;
	}

	return 0;
}

void pure64_config_init(struct pure64_config *config) {
	config->arch = PURE64_ARCH_NONE;
	config->bootsector = PURE64_BOOTSECTOR_NONE;
	config->partition_scheme = PURE64_PARTITION_SCHEME_NONE;
	config->stage_three = PURE64_STAGE_THREE_NONE;
	config->disk_size = 0;
	config->kernel = NULL;
}

void pure64_config_done(struct pure64_config *config) {
	free(config->kernel);
	config->kernel = NULL;
}

int pure64_config_parse(struct pure64_config *config,
                        const char *source,
                        struct pure64_config_error *error) {

	unsigned int i = 0;
	unsigned int line_number = 1;

	while (source[i] != 0) {

		const char *line = &source[i];

		unsigned int line_size = 0;

		/* get line size */
		while ((source[i] != 0) && (source[i] != '\n')) {
			i++;
			line_size++;
		}

		if (source[i] == '\n')
			i++;
		else if ((source[i] == '\r')
		      && (source[i] == '\n'))
			i += 2;
		else if ((source[i] == '\n')
		      && (source[i] == '\r'))
			i += 2;

		if (line_size == 0)
			continue;
		else if (is_blank(line, line_size))
			continue;
		else if (is_comment(line, line_size))
			continue;

		struct pure64_var var;

		int err = parse_var(&var, line, line_size, error);
		if (err != 0) {
			error->line = line_number;
			return err;
		}

		err = handle_var(config, &var, error);
		if (err != 0) {
			error->line = line_number;
			return err;
		}

		line_number++;
	}

	int err = validate_vars(config, error);
	if (err != 0) {
		error->line = 0;
		return err;
	}

	return 0;
}

int pure64_config_load(struct pure64_config *config,
                       const char *filename,
                       struct pure64_config_error *error) {

	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		error->desc = "Failed to open file";
		return PURE64_EINVAL;
	}

	if (fseek(file, 0UL, SEEK_END) != 0) {
		fclose(file);
		error->desc = "Failed to seek to end of file";
		return PURE64_EINVAL;
	}

	long int file_size = ftell(file);
	if (file_size == -1L) {
		fclose(file);
		error->desc = "Failed to tell file position";
		return PURE64_EINVAL;
	}

	if (fseek(file, 0UL, SEEK_SET) != 0) {
		fclose(file);
		error->desc = "Failed to seek to start of file";
		return PURE64_EINVAL;
	}

	char *source = malloc(file_size + 1);
	if (source == NULL) {
		fclose(file);
		error->desc = "Failed to allocate file memory";
		return PURE64_ENOMEM;
	}

	source[file_size] = 0;

	if (fread(source, 1, file_size, file) != ((size_t) file_size)) {
		free(source);
		fclose(file);
		error->desc = "Failed to read file";
		return PURE64_EIO;
	}

	fclose(file);

	int err = pure64_config_parse(config, source, error);

	free(source);

	return err;
}
