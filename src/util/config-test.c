#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_parse(void) {

	const char source[] = "arch: x86_64\n"
	                      "partition_scheme: gpt\n"
	                      "bootsector: mbr\n"
	                      "disk_size: 2M\n"
	                      "stage_three: loader\n";

	struct pure64_config config;

	struct pure64_config_error error;

	pure64_config_init(&config);

	int err = pure64_config_parse(&config, source, &error);

	if (err != 0) {
		fprintf(stderr, "<source>:%lu:%lu: %s\n", error.line, error.column, error.desc);
	}

	assert(err == 0);
	assert(config.arch == PURE64_ARCH_x86_64);
	assert(config.bootsector == PURE64_BOOTSECTOR_MBR);
	assert(config.partition_scheme == PURE64_PARTITION_SCHEME_GPT);
	assert(config.stage_three == PURE64_STAGE_THREE_LOADER);
	assert(config.disk_size == (2 * 1024 * 1024));
}

int main(void) {
	test_parse();
	return EXIT_SUCCESS;
}
