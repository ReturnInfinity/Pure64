/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/lang/var.h>

#include <pure64/core/error.h>
#include <pure64/core/string.h>

#include <pure64/lang/scanner.h>
#include <pure64/lang/syntax-error.h>
#include <pure64/lang/token.h>

#include <stdlib.h>

void pure64_value_init(struct pure64_value *value) {
	value->type = PURE64_TYPE_null;
	value->column = 1;
	value->line = 1;
}

void pure64_value_done(struct pure64_value *value) {
	/* TODO */
	value->column = 1;
	value->line = 1;
}

void pure64_key_init(struct pure64_key *key) {
	key->id = pure64_null;
	key->id_size = 0;
	key->column = 1;
	key->line = 1;
}

void pure64_key_done(struct pure64_key *key) {
	key->id = pure64_null;
	key->id_size = 0;
	key->column = 1;
	key->line = 1;
}

int pure64_key_cmp_id(struct pure64_key *key,
                      const char *id) {

	if (key->id == pure64_null) {
		if ((id == pure64_null) || (id[0] == 0))
			return 0;
		else
			return -1;
	} else if (id == pure64_null) {
		if (key->id[0] == 0)
			return 0;
		else
			return 1;
	}

	unsigned int i = 0;

	while ((i < key->id_size) && (id[i] != 0)) {
		if (key->id[i] > id[i])
			return 1;
		else if (key->id[i] < id[i])
			return -1;
		else
			i++;
	}

	if (i < key->id_size)
		return 1;
	else if (id[i] != 0)
		return -1;
	else
		return 0;
}

int pure64_key_parse(struct pure64_key *key,
                     struct pure64_scanner *scanner,
                     struct pure64_syntax_error *error) {

	pure64_size original_index = scanner->index;

	const struct pure64_token *token = pure64_scanner_next(scanner);
	if (token == pure64_null) {
		if (error != pure64_null) {
			error->source = pure64_null;
			error->desc = "End of file reached.";
			error->line = 0;
			error->column = 0;
		}
		scanner->index = original_index;
		return PURE64_EINVAL;
	}

	if ((token->type == PURE64_TOKEN_IDENTIFIER)
	 || (token->type == PURE64_TOKEN_SINGLE_QUOTE)
	 || (token->type == PURE64_TOKEN_DOUBLE_QUOTE)) {
		key->id = token->data;
		key->id_size = token->size;
	} else {
		if (error != pure64_null) {
			error->source = pure64_null;
			error->desc = "Expected an identifier or string.";
			error->line = token->line;
			error->column = token->column;
		}
		scanner->index = original_index;
		return PURE64_EINVAL;
	}

	return 0;
}

int pure64_key_parse_s(struct pure64_key *key,
                       const char *source,
                       struct pure64_syntax_error *error) {

	struct pure64_scanner scanner;
	pure64_scanner_init(&scanner);
	pure64_scanner_reject_whitespace(&scanner);
	pure64_scanner_reject_comments(&scanner);

	int err = pure64_scanner_scan(&scanner, source);
	if (err != 0) {
		pure64_scanner_done(&scanner);
		return err;
	}

	err = pure64_key_parse(key, &scanner, error);
	if (err != 0) {
		pure64_scanner_done(&scanner);
		return err;
	}

	pure64_scanner_done(&scanner);

	return 0;
}

void pure64_var_init(struct pure64_var *var) {
	pure64_key_init(&var->key);
	pure64_value_init(&var->value);
}

void pure64_var_done(struct pure64_var *var) {
	pure64_key_done(&var->key);
	pure64_value_done(&var->value);
}

int pure64_var_parse(struct pure64_var *var,
                     struct pure64_scanner *scanner,
                     struct pure64_syntax_error *error) {
	(void) var;
	(void) scanner;
	(void) error;
	return PURE64_ENOSYS;
}
