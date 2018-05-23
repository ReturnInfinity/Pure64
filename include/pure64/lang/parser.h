/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_LANG_PARSER_H
#define PURE64_LANG_PARSER_H

#include <pure64/core/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_syntax_error;
struct pure64_var;

/** Used for parsing variables.
 * @ingroup lang-api
 * */

struct pure64_parser {
	/** The variable array. */
	struct pure64_var *var_array;
	/** The number of variables in
	 * variable array. */
	pure64_size var_count;
	/** The current variable index. */
	pure64_size var_index;
};

/** Initializes a parser structure.
 * @param parser The parser structure
 * to initialize.
 * @ingroup lang-api
 * */

void pure64_parser_init(struct pure64_parser *parser);

/** Releases memory allocated by a parser structure.
 * @param parser An initialized parser structure.
 * @ingroup lang-api
 * */

void pure64_parser_done(struct pure64_parser *parser);

/** Rewinds the parser to the first variable
 * that was parsed.
 * @param parser An initialized parser structure.
 * @ingroup lang-api
 * */

void pure64_parser_begin(struct pure64_parser *parser);

/** Indicates when the last variable
 * has been reached by the caller.
 * @param parser An initialized parser structure.
 * @returns @ref pure64_true if the last variable
 * was reached, @ref pure64_false if it was not.
 * @ingroup lang-api
 * */

pure64_bool pure64_parser_eof(const struct pure64_parser *parser);

/** Goes to the next variable found by the parser.
 * @param parser An initialized parser structure.
 * @returns A pointer to the next variable found
 * by the parser. If the last variable was already
 * reached, then @ref pure64_null is returned.
 * @ingroup lang-api
 * */

const struct pure64_var *pure64_parser_next(struct pure64_parser *parser);

/** Parses a configuration file source.
 * @param parser An initialized parser structure.
 * @param source The source to parse.
 * @param error A pointer to an error structure
 * that may be used to indicate the location
 * of a syntax error.
 * @returns Zero on success, an error code on failure.
 * The error code, @ref PURE64_EINVAL is returned on
 * syntax errors.
 * @ingroup lang-api
 * */

int pure64_parser_parse(struct pure64_parser *parser,
                        const char *source,
                        struct pure64_syntax_error *error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PURE64_LANG_PARSER_H */
