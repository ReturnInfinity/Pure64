/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/lang/parser.h>

#include <pure64/core/error.h>
#include <pure64/lang/scanner.h>
#include <pure64/lang/syntax-error.h>
#include <pure64/lang/token.h>
#include <pure64/lang/var.h>

#include <stdlib.h>

void pure64_parser_init(struct pure64_parser *parser) {
	parser->var_array = pure64_null;
	parser->var_count = 0;
	parser->var_index = 0;
}

void pure64_parser_done(struct pure64_parser *parser) {

	for (pure64_size i = 0; i < parser->var_count; i++)
		pure64_var_done(&parser->var_array[i]);

	free(parser->var_array);

	parser->var_array = pure64_null;
	parser->var_count = 0;
	parser->var_index = 0;
}

pure64_bool pure64_parser_eof(const struct pure64_parser *parser) {
	if (parser->var_index >= parser->var_count)
		return pure64_true;
	else
		return pure64_false;
}

const struct pure64_var *pure64_parser_next(struct pure64_parser *parser) {
	if (pure64_parser_eof(parser))
		return pure64_null;
	else
		return &parser->var_array[parser->var_index];
}

static int push_var(struct pure64_parser *parser,
                    struct pure64_var *var) {
	(void) parser;
	(void) var;
	return PURE64_ENOSYS;
}

int pure64_parser_parse(struct pure64_parser *parser,
                        const char *source,
                        struct pure64_syntax_error *error) {

	struct pure64_scanner scanner;
	pure64_scanner_init(&scanner);
	pure64_tokenbuf_reject_whitespace(scanner.tokenbuf);
	pure64_tokenbuf_reject_comments(scanner.tokenbuf);

	int err = pure64_scanner_scan(&scanner, source);
	if (err != 0) {
		if (error != pure64_null) {
			error->source = "<string>";
			error->desc = "Failed to tokenize source.";
			error->line = 1;
			error->column = 1;
		}
		return err;
	}

	while (!pure64_scanner_eof(&scanner)) {

		struct pure64_var var;

		pure64_var_init(&var);

		err = pure64_var_parse(&var, &scanner, error);
		if (err != 0) {
			pure64_scanner_done(&scanner);
			return err;
		}

		err = push_var(parser, &var);
		if (err != 0) {
			pure64_var_done(&var);
			pure64_scanner_done(&scanner);
			return err;
		}
	}

	pure64_scanner_done(&scanner);

	return 0;
}
