#include "config.h"

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
                     unsigned int line_size) {

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
			/* TODO error message: expected a colon */
			return -1;
		}
		i++;
	}

	if (i >= line_size) {
		/* TODO error message : expected value
		 * following variable key */
		return -1;
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

static int handle_var(struct pure64_config *config,
                      const struct pure64_var *var) {

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
			/* TODO error message : unknown bootsector */
			return -1;
		}
	} else if (is_key(var, "partition-scheme")) {
		if (is_value(var, "none")) {
			config->partition_scheme = PURE64_PARTITION_SCHEME_NONE;
		} else if (is_value(var, "gpt")) {
			config->partition_scheme = PURE64_PARTITION_SCHEME_GPT;
		} else {
			/* TODO error message : unknown partition scheme */
			return -1;
		}
	} else {
		/* TODO error message : unknown key */
		return -1;
	}

	return 0;
}

void pure64_config_init(struct pure64_config *config) {
	config->bootsector = PURE64_BOOTSECTOR_PXE;
	config->partition_scheme = PURE64_PARTITION_SCHEME_NONE;
}

int pure64_config_parse(struct pure64_config *config,
                        const char *source) {

	unsigned int i = 0;

	while (source[i] != 0) {

		const char *line = &source[i];

		unsigned int line_size = 0;

		/* get line size */
		while ((source[i] != 0) && (source[i] != '\n')) {
			i++;
			line_size++;
		}

		i++;

		struct pure64_var var;

		int err = parse_var(&var, line, line_size);
		if (err != 0) {
			return err;
		}

		err = handle_var(config, &var);
		if (err != 0) {
			return err;
		}
	}

	return 0;
}
