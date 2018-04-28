#include <pure64/lang/token.h>

#include <pure64/error.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/** The position of the scanner
 * within the source code.
 * */

struct source_pos {
	/** Indicates the line number, starting at one. */
	unsigned long int line;
	/** Indicates the column, starting at one. */
	unsigned long int column;
};

static void source_pos_init(struct source_pos *pos) {
	pos->line = 1;
	pos->column = 1;
}

static void source_pos_update(struct source_pos *pos,
                              const char *str,
                              unsigned long int size) {

	unsigned long int i = 0;

	while (i < size) {
		char c = str[i];
		if (c == '\n') {
			pos->line++;
			pos->column = 1;
		} else {
			pos->column++;
		}
		i++;
	}
}

const struct pure64_token pure64_eof_token = {
	PURE64_TOKEN_END /* token type */,
	NULL /* data */,
	0 /* size */,
	0 /* width */,
	0 /* line */,
	0 /* column */
};

static int parse_whitespace(struct pure64_token *token, const char *source) {

	unsigned long int i = 0;

	while (source[i] != 0) {
		if ((source[i] != ' ')
		 && (source[i] != '\t')
		 && (source[i] != '\r')
		 && (source[i] != '\n')) {
			break;
		}
		i++;
	}

	if (i > 0) {
		token->type = PURE64_TOKEN_WHITESPACE;
		token->data = source;
		token->size = i;
		token->width = i;
		return 0;
	}

	return PURE64_EINVAL;
}

static int parse_comment(struct pure64_token *token, const char *source) {

	if (source[0] != '#') {
		return PURE64_EINVAL;
	}

	unsigned long int i = 1;

	while (source[i] != 0) {
		if (source[i] == '\n')
			break;
		i++;
	}

	token->type = PURE64_TOKEN_COMMENT;
	token->data = &source[1];
	token->size = i - 1;
	token->width = i;

	return 0;
}

static int parse_single_quote(struct pure64_token *token, const char *source) {

	unsigned long int i = 0;

	if (source[i] != '\'') {
		return PURE64_EINVAL;
	}

	for (i = 1; source[i] != 0; i++) {
		if (source[i] == '\'') {
			token->type = PURE64_TOKEN_SINGLE_QUOTE;
			token->data = &source[1];
			token->size = i - 1;
			token->width = token->size + 2;
			return 0;
		}
	}

	return PURE64_EINVAL;
}

static int parse_double_quote(struct pure64_token *token, const char *source) {

	unsigned long int i = 0;

	if (source[i] != '\"') {
		return PURE64_EINVAL;
	}

	for (i = 1; source[i] != 0; i++) {
		if (source[i] == '\"') {
			token->type = PURE64_TOKEN_DOUBLE_QUOTE;
			token->data = &source[1];
			token->size = i - 1;
			token->width = token->size + 2;
			return 0;
		}
	}

	return PURE64_EINVAL;
}

static int parse_colon(struct pure64_token *token, const char *source) {

	if (source[0] == ':') {
		token->type = PURE64_TOKEN_COLON;
		token->data = source;
		token->size = 1;
		token->width = 1;
		return 0;
	}

	return PURE64_EINVAL;
}

static int parse_identifier(struct pure64_token *token, const char *source) {

	char c = source[0];

	if (!isalpha(c) && (c != '_')) {
		return PURE64_EINVAL;
	}

	unsigned int i = 1;

	while (source[i] != 0) {
		if (!isalnum(source[i]) && (source[i] != '_')) {
			break;
		}
		i++;
	}

	token->type = PURE64_TOKEN_IDENTIFIER;
	token->data = source;
	token->size = i;
	token->width = i;

	return 0;
}

static int parse_numerical(struct pure64_token *token, const char *source) {

	char c = source[0];
	if (!((c >= '0') && (c <= '9'))) {
		return PURE64_EINVAL;
	}

	unsigned int i = 1;

	while (source[i] != 0) {
		if (!isalnum(source[i]) && (source[i] != '_')) {
			break;
		}
		i++;
	}

	token->type = PURE64_TOKEN_IDENTIFIER;
	token->data = source;
	token->size = i;
	token->width = i;

	return 0;
}

static int reserve_tokens(struct pure64_tokenbuf *tokenbuf) {

	unsigned long int next_size = 0;
	next_size += tokenbuf->tokens_reserved;
	next_size += 64;
	next_size *= sizeof(struct pure64_token);

	struct pure64_token *tmp = tokenbuf->token_array;

	tmp = realloc(tmp, next_size);
	if (tmp == NULL)
		return PURE64_ENOMEM;

	tokenbuf->token_array = tmp;
	tokenbuf->tokens_reserved += 64;

	return 0;
}

void pure64_token_init(struct pure64_token *token) {
	token->type = PURE64_TOKEN_NONE;
	token->data = NULL;
	token->size = 0;
	token->width = 0;
	token->line = 1;
	token->column = 1;
}

int pure64_token_parse(struct pure64_token *token, const char *source) {

	if ((parse_whitespace(token, source) == 0)
	 || (parse_comment(token, source) == 0)
	 || (parse_single_quote(token, source) == 0)
	 || (parse_double_quote(token, source) == 0)
	 || (parse_colon(token, source) == 0)
	 || (parse_identifier(token, source) == 0)
	 || (parse_numerical(token, source) == 0)) {
		return 0;
	}

	if (source[0] == 0) {
		token->type = PURE64_TOKEN_END;
		token->data = source;
		token->size = 0;
		token->width = 0;
	} else {
		token->type = PURE64_TOKEN_UNKNOWN;
		token->data = source;
		token->size = 1;
		token->width = 1;
	}

	return 0;
}

void pure64_tokenbuf_init(struct pure64_tokenbuf *tokenbuf) {
	tokenbuf->token_array = NULL;
	tokenbuf->token_count = 0;
	tokenbuf->tokens_reserved = 0;
	tokenbuf->allowing_comments = 1;
	tokenbuf->allowing_whitespace = 1;
}

void pure64_tokenbuf_done(struct pure64_tokenbuf *tokenbuf) {
	free(tokenbuf->token_array);
	tokenbuf->token_array = NULL;
	tokenbuf->token_count = 0;
	tokenbuf->tokens_reserved = 0;
}

void pure64_tokenbuf_accept_comments(struct pure64_tokenbuf *tokenbuf) {
	tokenbuf->allowing_comments = 1;
}

void pure64_tokenbuf_accept_whitespace(struct pure64_tokenbuf *tokenbuf) {
	tokenbuf->allowing_whitespace = 1;
}

void pure64_tokenbuf_reject_comments(struct pure64_tokenbuf *tokenbuf) {
	tokenbuf->allowing_comments = 0;
}

void pure64_tokenbuf_reject_whitespace(struct pure64_tokenbuf *tokenbuf) {
	tokenbuf->allowing_whitespace = 0;
}

int pure64_tokenbuf_parse(struct pure64_tokenbuf *tokenbuf, const char *source) {

	unsigned long int i = 0;

	unsigned long int source_len = strlen(source);

	struct pure64_token token;

	struct source_pos pos;

	source_pos_init(&pos);

	while (i < source_len) {

		pure64_token_init(&token);

		int err = pure64_token_parse(&token, &source[i]);
		if (err != 0)
			return err;

		token.line = pos.line;
		token.column = pos.column;

		source_pos_update(&pos, &source[i], token.width);

		if ((token.type == PURE64_TOKEN_WHITESPACE)
		 && (!tokenbuf->allowing_whitespace)) {
			i += token.width;
			continue;
		}

		if ((token.type == PURE64_TOKEN_COMMENT)
		 && (!tokenbuf->allowing_comments)) {
			i += token.width;
			continue;
		}

		err = pure64_tokenbuf_push(tokenbuf, &token);
		if (err != 0)
			return err;

		i += token.width;
	}

	int err = pure64_tokenbuf_push(tokenbuf, &pure64_eof_token);
	if (err != 0)
		return err;

	return 0;
}

int pure64_tokenbuf_push(struct pure64_tokenbuf *tokenbuf, const struct pure64_token *token) {

	if ((tokenbuf->token_count + 1) >= tokenbuf->tokens_reserved) {
		int err = reserve_tokens(tokenbuf);
		if (err != 0)
			return err;
	}

	tokenbuf->token_array[tokenbuf->token_count++] = *token;

	return 0;
}
