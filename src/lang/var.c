/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/lang/var.h>

void pure64_value_init(struct pure64_value *value) {
	value->type = PURE64_TYPE_null;
	value->column = 1;
	value->line = 1;
}

void pure64_key_init(struct pure64_key *key) {
	key->id = pure64_null;
	key->id_size = 0;
	key->column = 1;
	key->line = 1;
}

void pure64_var_init(struct pure64_var *var) {
	pure64_key_init(&var->key);
	pure64_value_init(&var->value);
}
