#include "config.h"

#include "mbr-data.h"
#include "multiboot-data.h"
#include "multiboot2-data.h"
#include "pxe-data.h"

#include "token.h"

#include <pure64/error.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Used for iterating a token
 * buffer.
 * */

struct token_iterator {
	/** The token buffer. */
	struct pure64_tokenbuf *tokenbuf;
	/** The token index within the token buffer. */
	unsigned long int index;
};

static unsigned char token_iterator_end(const struct token_iterator *it) {
	if (it->index >= it->tokenbuf->token_count)
		return 1;
	else if (it->tokenbuf->token_array[it->index].type == PURE64_TOKEN_END)
		return 1;
	else
		return 0;
}

static void token_iterator_next(struct token_iterator *it) {
	if (!token_iterator_end(it))
		it->index++;
}

static const struct pure64_token *token_iterator_deref(const struct token_iterator *it) {
	if (!token_iterator_end(it))
		return &it->tokenbuf->token_array[it->index];
	else
		return &pure64_eof_token;
}

struct pure64_var {
	const char *key;
	unsigned long int key_size;
	const char *value;
	unsigned long int value_size;
};

static void pure64_var_init(struct pure64_var *var) {
	var->key = NULL;
	var->key_size = 0;
	var->value = NULL;
	var->value_size = 0;
}

static int pure64_var_parse(struct pure64_var *var,
                            struct token_iterator *it,
                            struct pure64_config_error *error) {

	const struct pure64_token *var_key = token_iterator_deref(it);
	if ((var_key->type != PURE64_TOKEN_IDENTIFIER)
	 && (var_key->type != PURE64_TOKEN_SINGLE_QUOTE)
	 && (var_key->type != PURE64_TOKEN_DOUBLE_QUOTE)) {
		error->desc = "Expected an identifier";
		error->line = var_key->line;
		error->column = var_key->column;
		return PURE64_EINVAL;
	}

	var->key = var_key->data;
	var->key_size = var_key->size;

	token_iterator_next(it);

	const struct pure64_token *colon = token_iterator_deref(it);
	if (colon->type != PURE64_TOKEN_COLON) {
		error->desc = "Expected a ':'";
		error->line = colon->line;
		error->column = colon->column;
		return PURE64_EINVAL;
	}

	token_iterator_next(it);

	const struct pure64_token *var_value = token_iterator_deref(it);
	if ((var_value->type != PURE64_TOKEN_IDENTIFIER)
	 && (var_value->type != PURE64_TOKEN_SINGLE_QUOTE)
	 && (var_value->type != PURE64_TOKEN_DOUBLE_QUOTE)
	 && (var_value->type != PURE64_TOKEN_NUMERICAL)) {
		error->desc = "Expected a string or a numerical";
		error->line = var_value->line;
		error->column = var_value->column;
		return PURE64_EINVAL;
	}

	var->value = var_value->data;
	var->value_size = var_value->size;

	token_iterator_next(it);

	return 0;
}

static unsigned char pure64_var_cmp_key(const struct pure64_var *var,
                                        const char *key) {

	unsigned long int key_size = strlen(key);

	if (key_size != var->key_size)
		return 0;
	else if (memcmp(var->key, key, key_size) != 0)
		return 0;
	else
		return 1;
}

static unsigned char pure64_var_cmp_value(const struct pure64_var *var,
                                          const char *value) {

	unsigned long int value_size = strlen(value);

	if (value_size != var->value_size)
		return 0;
	else if (memcmp(var->value, value, value_size) != 0)
		return 0;
	else
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
	if (tmp == NULL) {
		return PURE64_ENOMEM;
	}

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
		return PURE64_EINVAL;
	}

	config->disk_size *= multiplier;

	free(tmp);

	return 0;
}

static int handle_var(struct pure64_config *config,
                      const struct pure64_var *var,
                      struct pure64_config_error *error) {

	if (pure64_var_cmp_key(var, "bootsector")) {
		if (pure64_var_cmp_value(var, "mbr")) {
			config->bootsector = PURE64_BOOTSECTOR_MBR;
		} else if (pure64_var_cmp_value(var, "pxe")) {
			config->bootsector = PURE64_BOOTSECTOR_PXE;
		} else if (pure64_var_cmp_value(var, "multiboot")) {
			config->bootsector = PURE64_BOOTSECTOR_MULTIBOOT;
		} else if (pure64_var_cmp_value(var, "multiboot2")) {
			config->bootsector = PURE64_BOOTSECTOR_MULTIBOOT2;
		} else {
			error->desc = "Unknown bootsector type";
			return PURE64_EINVAL;
		}
	} else if (pure64_var_cmp_key(var, "kernel_path")) {
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
			return PURE64_ENOENT;
		}
	} else if (pure64_var_cmp_key(var, "stage_three")) {
		if (pure64_var_cmp_value(var, "kernel")) {
			config->stage_three = PURE64_STAGE_THREE_KERNEL;
		} else if (pure64_var_cmp_value(var, "loader")) {
			config->stage_three = PURE64_STAGE_THREE_LOADER;
		} else {
			error->desc = "Unknown stage three type";
			return PURE64_EINVAL;
		}
	} else if (pure64_var_cmp_key(var, "partition_scheme")) {
		if (pure64_var_cmp_value(var, "none")) {
			config->partition_scheme = PURE64_PARTITION_SCHEME_NONE;
		} else if (pure64_var_cmp_value(var, "gpt")) {
			config->partition_scheme = PURE64_PARTITION_SCHEME_GPT;
		} else {
			error->desc = "Unknown partition scheme";
			return PURE64_EINVAL;
		}
	} else if (pure64_var_cmp_key(var, "disk_size")) {
		if (var->value_size == 0) {
			error->desc = "Size not specified";
			return -1;
		}
		if (parse_disk_size(config, var->value, var->value_size) != 0) {
			error->desc = "Invalid size";
			return PURE64_EINVAL;
		}
	} else if (pure64_var_cmp_key(var, "arch")) {
		if (pure64_var_cmp_value(var, "x86_64")) {
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

	struct pure64_tokenbuf tokenbuf;

	pure64_tokenbuf_init(&tokenbuf);
	pure64_tokenbuf_reject_comments(&tokenbuf);
	pure64_tokenbuf_reject_whitespace(&tokenbuf);

	int err = pure64_tokenbuf_parse(&tokenbuf, source);
	if (err != 0) {
		error->desc = "Failed to scan source";
		error->line = 0;
		pure64_tokenbuf_done(&tokenbuf);
		return err;
	}

	struct token_iterator it;
	it.index = 0;
	it.tokenbuf = &tokenbuf;

	while (!token_iterator_end(&it)) {

		struct pure64_var var;

		pure64_var_init(&var);

		err = pure64_var_parse(&var, &it, error);
		if (err != 0) {
			pure64_tokenbuf_done(&tokenbuf);
			return err;
		}
		
		err = handle_var(config, &var, error);
		if (err != 0) {
			pure64_tokenbuf_done(&tokenbuf);
			return err;
		}
	}

	pure64_tokenbuf_done(&tokenbuf);

	err = validate_vars(config, error);
	if (err != 0) {
		error->line = 0;
		error->column = 0;
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
		error->line = 0;
		return PURE64_EINVAL;
	}

	if (fseek(file, 0UL, SEEK_END) != 0) {
		fclose(file);
		error->desc = "Failed to seek to end of file";
		error->line = 0;
		return PURE64_EINVAL;
	}

	long int file_size = ftell(file);
	if (file_size == -1L) {
		fclose(file);
		error->desc = "Failed to tell file position";
		error->line = 0;
		return PURE64_EINVAL;
	}

	if (fseek(file, 0UL, SEEK_SET) != 0) {
		fclose(file);
		error->desc = "Failed to seek to start of file";
		error->line = 0;
		return PURE64_EINVAL;
	}

	char *source = malloc(file_size + 1);
	if (source == NULL) {
		fclose(file);
		error->desc = "Failed to allocate file memory";
		error->line = 0;
		return PURE64_ENOMEM;
	}

	source[file_size] = 0;

	if (fread(source, 1, file_size, file) != ((size_t) file_size)) {
		free(source);
		fclose(file);
		error->desc = "Failed to read file";
		error->line = 0;
		return PURE64_EIO;
	}

	fclose(file);

	int err = pure64_config_parse(config, source, error);

	free(source);

	return err;
}
