/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_SCANNER_H
#define PURE64_SCANNER_H

#include <pure64/core/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_token;
struct pure64_tokenbuf;

/** A token scanner. Contains a token buffer
 * and allows the caller to go through the tokens
 * in the token buffer in a clean manner.
 * @ingroup lang-api
 * */

struct pure64_scanner {
	/** The token buffer. */
	struct pure64_tokenbuf *tokenbuf;
	/** The token index within the token buffer. */
	pure64_size index;
};

/** Initializes the scanner structure.
 * @param scanner The scanner structure to initialize.
 * @ingroup lang-api
 * */

void pure64_scanner_init(struct pure64_scanner *scanner);

/** Releases memory allocated by the scanner structure.
 * @param scanner An initialized scanner structure.
 * @ingroup lang-api
 * */

void pure64_scanner_done(struct pure64_scanner *scanner);

/** Discards comments when tokenizing the source.
 * @param scanner An initialized scanner structure.
 * @ingroup lang-api
 * */

void pure64_scanner_reject_comments(struct pure64_scanner *scanner);

/** Discards whitespace when tokenizing the source.
 * @param scanner An initialized scanner structure.
 * @ingroup lang-api
 * */

void pure64_scanner_reject_whitespace(struct pure64_scanner *scanner);

/** Indicates when the scanner has no more tokens left.
 * @param scanner An initialized scanner structure.
 * @returns @ref pure64_true if the scanner has no more
 * tokens left, @ref pure64_false if it does.
 * @ingroup lang-api
 * */

pure64_bool pure64_scanner_eof(const struct pure64_scanner *scanner);

/** Tokenizes the source code, appending new tokens to the
 * end of the token buffer.
 * @param scanner An initialized scanner structure.
 * @param source A null-terminated string containing
 * the source code to tokenize.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_scanner_scan(struct pure64_scanner *scanner,
                        const char *source);

/** Resets the token index back to zero.
 * This is useful if the source code needs to be
 * scanned from the beginning again. It can also
 * be used just to ensure that the scanner starts
 * at the first token.
 * @param scanner An initialized scanner structure.
 * @ingroup lang-api
 * */

void pure64_scanner_begin(struct pure64_scanner *scanner);

/** Returns the next token in the scanner's token
 * buffer. If no more tokens are left over, then
 * @ref pure64_null is returned.
 * @param scanner An initialized scanner structure.
 * @returns A pointer to the next scanner token on
 * success, @ref pure64_null if there are no more left.
 * @ingroup lang-api
 * */

const struct pure64_token *pure64_scanner_next(struct pure64_scanner *scanner);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_SCANNER_H */
