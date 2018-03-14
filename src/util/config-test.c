#include "config.h"

#include <assert.h>
#include <stdlib.h>

static void test_parse(void) {

	const char source[] = "partition-scheme: gpt\n"
	                      "bootsector: mbr\n";

	struct pure64_config config;

	pure64_config_init(&config);

	assert(pure64_config_parse(&config, source) == 0);

	assert(config.bootsector == PURE64_BOOTSECTOR_MBR);
	assert(config.partition_scheme == PURE64_PARTITION_SCHEME_GPT);
}

int main(void) {
	test_parse();
	return EXIT_SUCCESS;
}
