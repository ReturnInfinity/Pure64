/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/lang/parser.h>
#include <pure64/lang/var.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void unit_test_key(const char *id, const char *source) {

	struct pure64_key key;

	pure64_key_init(&key);

	int err = pure64_key_parse_s(&key, source, pure64_null);
	assert(err == 0);

	assert(pure64_key_cmp_id(&key, id) == 0);

	pure64_key_done(&key);
}

static void test_key(void) {
	unit_test_key("k1", " k1 ");
	unit_test_key("k2", "\tk2\t");
	unit_test_key("k3", "\"k3\"");
	unit_test_key("k4", "\'k4\'");
}

static void test_parser(void) {

	const char source[] = " partitions : [       \n"
	                      "  {                   \n"
	                      "    name : rootfs,    \n"
	                      "    size : 2MiB       \n"
	                      "  },                  \n"
	                      "  {                   \n"
	                      "    name : null,      \n"
	                      "    size : null       \n"
	                      "  }                   \n"
	                      "]                     \n";

	struct pure64_parser parser;
	pure64_parser_init(&parser);

	int err = pure64_parser_parse(&parser, source, pure64_null);
	assert(err == 0);

	const struct pure64_var *var = pure64_parser_next(&parser);
	assert(var != pure64_null);

	var = pure64_parser_next(&parser);
	assert(var == pure64_null);

	pure64_parser_done(&parser);
}

int main(void) {
	test_key();
	test_parser();
	return EXIT_SUCCESS;
}
