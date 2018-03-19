#include "token.h"

#include <assert.h>
#include <stdlib.h>

static void test_parse(void) {

	const char *source = "_id01 : 'value 1'\n"
	                     "#comment\n"
	                     "_id02\t:\t\"value 2\"\n";

	struct pure64_tokenbuf tokenbuf;

	pure64_tokenbuf_init(&tokenbuf);
	pure64_tokenbuf_reject_comments(&tokenbuf);
	pure64_tokenbuf_reject_whitespace(&tokenbuf);

	int err = pure64_tokenbuf_parse(&tokenbuf, source);
	assert(err == 0);
	assert(tokenbuf.token_count == 7);
	assert(tokenbuf.token_array[0].type == PURE64_TOKEN_IDENTIFIER);
	assert(tokenbuf.token_array[1].type == PURE64_TOKEN_COLON);
	assert(tokenbuf.token_array[2].type == PURE64_TOKEN_SINGLE_QUOTE);
	assert(tokenbuf.token_array[3].type == PURE64_TOKEN_IDENTIFIER);
	assert(tokenbuf.token_array[4].type == PURE64_TOKEN_COLON);
	assert(tokenbuf.token_array[5].type == PURE64_TOKEN_DOUBLE_QUOTE);
	assert(tokenbuf.token_array[6].type == PURE64_TOKEN_END);

	pure64_tokenbuf_done(&tokenbuf);
}

int main(void) {
	test_parse();
	return EXIT_SUCCESS;
}
