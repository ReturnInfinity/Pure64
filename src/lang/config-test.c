/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/lang/config.h>
#include <pure64/lang/syntax-error.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_parse(void) {

	const char source[] = "arch: x86_64               \n"
	                      "partition_scheme: gpt      \n"
	                      "bootsector: mbr            \n"
	                      "disk_size: 2M              \n"
	                      "fs_loader: true            \n"
	                      "partitions: [              \n"
	                      "  { name: p1, size: 2K },  \n"
	                      "  { name: p2, size: 1M }   \n"
	                      "]                          \n";

	struct pure64_config config;

	struct pure64_syntax_error error;

	pure64_config_init(&config);

	int err = pure64_config_parse(&config, source, &error);

	if (err != 0) {
		fprintf(stderr, "<source>:%lu:%lu: %s\n", error.line, error.column, error.desc);
	}

	assert(err == 0);
	assert(config.arch == PURE64_ARCH_x86_64);
	assert(config.bootsector == PURE64_BOOTSECTOR_MBR);
	assert(config.partition_scheme == PURE64_PARTITION_SCHEME_GPT);
	assert(config.fs_loader);
	assert(config.disk_size == (2 * 1024 * 1024));

	assert(config.partition_count == 2);

	assert(config.partitions[0].name_size == 2);
	assert(memcmp(config.partitions[0].name, "p1", 2) == 0);
	assert(config.partitions[0].size == 2048);
	assert(config.partitions[0].size_specified);
	assert(!config.partitions[0].offset_specified);

	assert(config.partitions[1].name_size == 2);
	assert(memcmp(config.partitions[1].name, "p2", 2) == 0);
	assert(config.partitions[1].size == (1 * 1024 * 1024));
	assert(config.partitions[0].size_specified);
	assert(!config.partitions[0].offset_specified);
}

int main(void) {
	test_parse();
	return EXIT_SUCCESS;
}
