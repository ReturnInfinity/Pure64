/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_TOKEN_H
#define PURE64_TOKEN_H

#include <pure64/core/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Enumerates a series of possible
 * token types in a config file.
 * @ingroup lang-api
 * */

enum pure64_token_type {
	/** No token type was specified. */
	PURE64_TOKEN_NONE,
	/** An unknown character was found. */
	PURE64_TOKEN_UNKNOWN,
	/** Token indicates the end of the file. */
	PURE64_TOKEN_END,
	/** Token consists of whitespace, including
	 * newlines, spaces, and tabs. */
	PURE64_TOKEN_WHITESPACE,
	/** Token is a comment */
	PURE64_TOKEN_COMMENT,
	/** Token is a string made up of two single quotes. */
	PURE64_TOKEN_SINGLE_QUOTE,
	/** Token is a string made up of two double quotes. */
	PURE64_TOKEN_DOUBLE_QUOTE,
	/** Token is a single colon. */
	PURE64_TOKEN_COLON,
	/** Token is an identifier. */
	PURE64_TOKEN_IDENTIFIER,
	/** Token is a number .*/
	PURE64_TOKEN_NUMERICAL
};

/** Describes a token in a config
 * file source.
 * @ingroup lang-api
 * */

struct pure64_token {
	/** This indicates the token type. */
	enum pure64_token_type type;
	/** This contains the characters of the token. */
	const char *data;
	/** This indicates the number of characters in the token. */
	unsigned long int size;
	/** This indicates how many characters the token occupies
	 * in the source text. This member is useful for iterating
	 * the source code, because it can be used to increment the
	 * character index after a successfull parse. */
	unsigned long int width;
	/** The line in the source that the token starts at. */
	unsigned long int line;
	/** The column within the line that the token starts at. */
	unsigned long int column;
};

/** This initializes the token structure
 * to reasonable default values. This should
 * be called before a token is parsed.
 * @param token The token to initialize.
 * @ingroup lang-api
 * */

void pure64_token_init(struct pure64_token *token);

/** Converts the token to a string.
 * For tokens that are either double quoted
 * or single quoted strings, the quotes are
 * discarded.
 * @param token An initialized token structure.
 * @returns A string representation of the token.
 * This function will not return @ref pure64_null
 * if the token is empty, it will return an empty
 * string. @ref pure64_null is only returned on
 * a memory allocation failure.
 * @ingroup lang-api
 * */

char *pure64_token_to_string(const struct pure64_token *token);

/** This parses a token from source text.
 * @param token An initialized token structure.
 * @param source A null-terminated source text.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_token_parse(struct pure64_token *token, const char *source);

/** This is the end-of-file token. It is sometimes
 * returned by a function, instead of a null pointer,
 * when an index is out of bounds.
 * @ingroup lang-api
 * */

extern const struct pure64_token pure64_eof_token;

/** This is a token buffer, used for parsing
 * a series of tokens.
 * @ingroup lang-api
 * */

struct pure64_tokenbuf {
	/** The token array, terminated by an end of
	 * file token. */
	struct pure64_token *token_array;
	/** The number of tokens in the token array,
	 * including the end of file token. */
	unsigned long int token_count;
	/** The number of token slots allocated in
	 * the token array. */
	unsigned long int tokens_reserved;
	/** Indicates whether or not whitespace
	 * tokens are added when parsed. */
	unsigned char allowing_whitespace;
	/** Indicates whether or not comment
	 * tokens are added when parsed. */
	unsigned char allowing_comments;
};

/** Initializes a token buffer.
 * @param tokenbuf The token buffer to initialize.
 * @ingroup lang-api
 * */

void pure64_tokenbuf_init(struct pure64_tokenbuf *tokenbuf);

/** Frees memory of a token buffer.
 * @param tokenbuf An initialized token buffer.
 * @ingroup lang-api
 * */

void pure64_tokenbuf_done(struct pure64_tokenbuf *tokenbuf);

/** This function will allow comments to be added to the
 * buffer when source is being parsed.
 * @param tokenbuf An initialized token buffer.
 * @ingroup lang-api
 * */

void pure64_tokenbuf_accept_comments(struct pure64_tokenbuf *tokenbuf);

/** This function will allow whitespace to be added
 * to the buffer when source is being parsed.
 * @param tokenbuf An initialized token buffer.
 * @ingroup lang-api
 * */

void pure64_tokenbuf_accept_whitespace(struct pure64_tokenbuf *tokenbuf);

/** This function will disallow comments from being
 * added to the buffer when source is being parsed.
 * @param tokenbuf An initialized token buffer.
 * @ingroup lang-api
 * */

void pure64_tokenbuf_reject_comments(struct pure64_tokenbuf *tokenbuf);

/** This function will disallow whitespace from being
 * added to the buffer when source is being parsed.
 * @param tokenbuf An initialized token buffer.
 * @ingroup lang-api
 * */

void pure64_tokenbuf_reject_whitespace(struct pure64_tokenbuf *tokenbuf);

/** Parses tokens from a source.
 * @param tokenbuf An initialized token buffer.
 * @param source The source to parse tokens for.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_tokenbuf_parse(struct pure64_tokenbuf *tokenbuf, const char *source);

/** Pushes a token to the end of the token buffer.
 * @param tokenbuf An initialized token buffer.
 * @param token The token to push to the end of
 * the buffer. This should not be an end-of-file
 * token, or an unknown or 'none' token.
 * @returns Zero on success, an error code on failure.
 * @ingroup lang-api
 * */

int pure64_tokenbuf_push(struct pure64_tokenbuf *tokenbuf, const struct pure64_token *token);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_TOKEN_H */
