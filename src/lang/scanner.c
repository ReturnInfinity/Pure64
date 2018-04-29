/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/lang/scanner.h>

#include <pure64/lang/token.h>
#include <pure64/core/error.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void pure64_scanner_init(struct pure64_scanner *scanner) {

	scanner->tokenbuf = malloc(sizeof(struct pure64_tokenbuf));
	if (scanner->tokenbuf != NULL)
		pure64_tokenbuf_init(scanner->tokenbuf);

	scanner->index = 0;
}

void pure64_scanner_done(struct pure64_scanner *scanner) {
	if (scanner->tokenbuf != NULL) {
		pure64_tokenbuf_done(scanner->tokenbuf);
		free(scanner->tokenbuf);
		scanner->tokenbuf = NULL;
	}
	scanner->index = 0;
}

void pure64_scanner_reject_comments(struct pure64_scanner *scanner) {
	if (scanner->tokenbuf != NULL) {
		pure64_tokenbuf_reject_comments(scanner->tokenbuf);
	}
}

void pure64_scanner_reject_whitespace(struct pure64_scanner *scanner) {
	if (scanner->tokenbuf != NULL) {
		pure64_tokenbuf_reject_whitespace(scanner->tokenbuf);
	}
}

pure64_bool pure64_scanner_eof(const struct pure64_scanner *scanner) {

	if (scanner->tokenbuf == NULL)
		return pure64_true;

	if (scanner->index >= scanner->tokenbuf->token_count)
		return pure64_true;
	else
		return pure64_false;
}

void pure64_scanner_begin(struct pure64_scanner *scanner) {
	scanner->index = 0;
}

int pure64_scanner_scan(struct pure64_scanner *scanner,
                        const char *source) {

	if (scanner->tokenbuf == NULL)
		return PURE64_ENOMEM;

	return pure64_tokenbuf_parse(scanner->tokenbuf, source);
}

const struct pure64_token *pure64_scanner_next(struct pure64_scanner *scanner) {

	if (scanner->tokenbuf == NULL)
		return pure64_null;

	if (pure64_scanner_eof(scanner))
		return pure64_null;
	else
		return &scanner->tokenbuf->token_array[scanner->index];
}
