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

void pure64_object_init(struct pure64_object *object) {
	object->var_array = pure64_null;
	object->var_count = 0;
}

void pure64_object_done(struct pure64_object *object) {
	for (pure64_size i = 0; i < object->var_count; i++) {
		pure64_var_done(&object->var_array[i]);
	}
	free(object->var_array);
	object->var_array = NULL;
	object->var_count = 0;
}

int pure64_object_parse(struct pure64_object *object,
                        struct pure64_scanner *scanner,
                        struct pure64_syntax_error *error) {

	/* Check for '}' */

	pure64_size original_index = scanner->index;

	const struct pure64_token *bracket = pure64_scanner_next(scanner);
	if (bracket == pure64_null) {
		return PURE64_EFAULT;
	} else if ((bracket->type == PURE64_TOKEN_BRACKET)
	        && (bracket->data[0] == '}')) {
		return 0;
	} else {
		scanner->index = original_index;
	}

	for (;;) {

		/* Parse the variable. */

		struct pure64_var var;

		pure64_var_init(&var);

		int err = pure64_var_parse(&var, scanner, error);
		if (err != 0)
			return err;

		err = pure64_object_push(object, &var);
		if (err != 0)
			return err;

		/* Check for comma or bracket */

		original_index = scanner->index;

		const struct pure64_token *token = pure64_scanner_next(scanner);
		if (token == pure64_null)
			return PURE64_EFAULT;
		else if (token->type == PURE64_TOKEN_COMMA)
			continue;
		else if ((token->type == PURE64_TOKEN_BRACKET)
		      && (token->data[0] == '}'))
			break;
	}

	return 0;
}

int pure64_object_push(struct pure64_object *object,
                       struct pure64_var *var) {

	pure64_size var_count = object->var_count + 1;

	struct pure64_var *var_array = object->var_array;

	var_array = realloc(var_array, sizeof(var_array[0]) * var_count);
	if (var_array == NULL)
		return PURE64_ENOMEM;

	var_array[var_count - 1] = *var;

	object->var_array = var_array;
	object->var_count = var_count;

	return 0;
}

void pure64_list_init(struct pure64_list *list) {
	list->value_array = pure64_null;
	list->value_count = 0;
}

void pure64_list_done(struct pure64_list *list) {
	for (pure64_size i = 0; i < list->value_count; i++) {
		pure64_value_done(&list->value_array[i]);
	}
	free(list->value_array);
	list->value_array = NULL;
	list->value_count = 0;
}

int pure64_list_parse(struct pure64_list *list,
                      struct pure64_scanner *scanner,
                      struct pure64_syntax_error *error) {

	/* Check for ']' */

	pure64_size original_index = scanner->index;

	const struct pure64_token *bracket = pure64_scanner_next(scanner);
	if (bracket == pure64_null) {
		return PURE64_EFAULT;
	} else if ((bracket->type == PURE64_TOKEN_BRACKET)
	        && (bracket->data[0] == ']')) {
		return 0;
	} else {
		scanner->index = original_index;
	}

	for (;;) {

		/* Parse the value. */

		struct pure64_value value;

		pure64_value_init(&value);

		int err = pure64_value_parse(&value, scanner, error);
		if (err != 0)
			return err;

		err = pure64_list_push(list, &value);
		if (err != 0)
			return err;

		/* Check for comma or bracket */

		original_index = scanner->index;

		const struct pure64_token *token = pure64_scanner_next(scanner);
		if (token == pure64_null)
			return PURE64_EFAULT;
		else if (token->type == PURE64_TOKEN_COMMA)
			continue;
		else if ((token->type == PURE64_TOKEN_BRACKET)
		      && (token->data[0] == ']'))
			break;
	}

	return 0;
}

int pure64_list_push(struct pure64_list *list,
                     struct pure64_value *value) {

	pure64_size value_count = list->value_count + 1;

	struct pure64_value *value_array = list->value_array;

	value_array = realloc(value_array, sizeof(value_array[0]) * value_count);
	if (value_array == NULL)
		return PURE64_ENOMEM;

	value_array[value_count - 1] = *value;

	list->value_array = value_array;
	list->value_count = value_count;

	return 0;
}

void pure64_value_init(struct pure64_value *value) {
	value->type = PURE64_VALUE_null;
	value->column = 1;
	value->line = 1;
}

void pure64_value_done(struct pure64_value *value) {
	/* TODO */
	switch (value->type) {
	case PURE64_VALUE_null:
	case PURE64_VALUE_boolean:
	case PURE64_VALUE_number:
	case PURE64_VALUE_string:
		break;
	case PURE64_VALUE_list:
		pure64_list_done(&value->u.list);
		break;
	case PURE64_VALUE_object:
		pure64_object_done(&value->u.object);
		break;
	}
	value->type = PURE64_VALUE_null;
	value->column = 1;
	value->line = 1;
}

int pure64_value_parse(struct pure64_value *value,
                       struct pure64_scanner *scanner,
                       struct pure64_syntax_error *error) {

	const struct pure64_token *token = pure64_scanner_next(scanner);
	if (token == pure64_null) {
		if (error != pure64_null) {
			error->source = pure64_null;
			error->desc = "Unexpected end of file.";
			error->line = 0;
			error->column = 0;
		}
		return PURE64_EINVAL;
	} else if (token->type == PURE64_TOKEN_IDENTIFIER) {
		if ((token->size == 4)
		 && (token->data[0] == 't')
		 && (token->data[1] == 'r')
		 && (token->data[2] == 'u')
		 && (token->data[3] == 'e')) {
			value->type = PURE64_VALUE_boolean;
			value->u._bool = pure64_true;
		} else if ((token->size == 4)
		        && (token->data[0] == 'n')
		        && (token->data[1] == 'u')
		        && (token->data[2] == 'l')
		        && (token->data[3] == 'l')) {
			value->type = PURE64_VALUE_null;
		} else if ((token->size == 5)
		        && (token->data[0] == 'f')
		        && (token->data[1] == 'a')
		        && (token->data[2] == 'l')
		        && (token->data[3] == 's')
		        && (token->data[4] == 'e')) {
			value->type = PURE64_VALUE_boolean;
			value->u._bool = pure64_false;
		} else {
			value->type = PURE64_VALUE_string;
			value->u.string.data = token->data;
			value->u.string.size = token->size;
		}
	} else if ((token->type == PURE64_TOKEN_SINGLE_QUOTE)
	        || (token->type == PURE64_TOKEN_DOUBLE_QUOTE)) {
		value->type = PURE64_VALUE_string;
		value->u.string.data = token->data;
		value->u.string.size = token->size;
	} else if (token->type == PURE64_TOKEN_BRACKET) {
		if (token->data[0] == '{') {
			value->type = PURE64_VALUE_object;
			pure64_object_init(&value->u.object);
			return pure64_object_parse(&value->u.object, scanner, error);
		} else if (token->data[0] == '[') {
			value->type = PURE64_VALUE_list;
			pure64_list_init(&value->u.list);
			return pure64_list_parse(&value->u.list, scanner, error);
		} else {
			if (error != pure64_null) {
				error->source = pure64_null;
				error->desc = "Unexpected symbol.";
				error->line = token->line;
				error->column = token->column;
			}
			return PURE64_EINVAL;
		}
	} else {
		if (error != pure64_null) {
			error->source = pure64_null;
			error->desc = "Unexpected token.";
			error->line = token->line;
			error->column = token->column;
		}
		return PURE64_EINVAL;
	}

	return 0;
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

int pure64_key_cmp_id(const struct pure64_key *key,
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

int pure64_var_cmp_id(const struct pure64_var *var,
                      const char *id) {

	return pure64_key_cmp_id(&var->key, id);
}

int pure64_var_parse(struct pure64_var *var,
                     struct pure64_scanner *scanner,
                     struct pure64_syntax_error *error) {

	int err = pure64_key_parse(&var->key, scanner, error);
	if (err != 0)
		return err;

	const struct pure64_token *colon_token = pure64_scanner_next(scanner);
	if (colon_token == pure64_null) {
		return PURE64_EFAULT;
	} else if (colon_token->type != PURE64_TOKEN_COLON) {
		if (error != pure64_null) {
			error->source = pure64_null;
			error->desc = "Expected a ':'.";
			error->line = colon_token->line;
			error->column = colon_token->column;
		}
		return PURE64_EINVAL;
	}

	err = pure64_value_parse(&var->value, scanner, error);
	if (err != 0)
		return err;

	return 0;
}
