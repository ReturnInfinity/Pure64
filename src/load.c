/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dir.h>
#include <pure64/file.h>

static bool find_kernel(struct pure64_file *kernel);

static void load_kernel(struct pure64_file *kernel);

void load(void) {

	struct pure64_file kernel;

	if (!find_kernel(&kernel)) {
		/* TODO :error message */
		return;
	}

	load_kernel(&kernel);
}

static void load_kernel(struct pure64_file *kernel) {
	(void) kernel;
}

static unsigned char *skip_file(unsigned char *ptr) {

	uint64_t *ptr64 = (uint64_t *) ptr;

	uint64_t name_size = ptr64[0];

	uint64_t data_size = ptr64[1];

	ptr = (unsigned char *) &ptr64[2];

	return &ptr[name_size + data_size];
}

static unsigned char *skip_dir(unsigned char *ptr) {

	uint64_t *ptr64 = (uint64_t *) ptr;

	uint64_t name_size = ptr64[0];

	uint64_t dir_count = ptr64[1];

	uint64_t file_count = ptr64[2];

	ptr = (unsigned char *) &ptr64[3];

	ptr = &ptr[name_size];

	for (uint64_t i = 0; i < dir_count; i++)
		ptr = skip_dir(ptr);
	
	for (uint64_t i = 0; i < file_count; i++)
		ptr = skip_file(ptr);

	return ptr;
}

static bool find_kernel(struct pure64_file *kernel) {

	/* The kernel resides in '/boot/kernel'.
	 * Find the 'boot' directory, then look
	 * for the 'kernel' file.
	 * */

	/* beginning of the directories in '/' */
	unsigned char *ptr = (unsigned char *) 0xa000;

	uint64_t i;
	uint64_t file_count;
	uint64_t dir_count;
	uint64_t name_size;

	dir_count = *(uint64_t *)(&ptr[0x08]);

	ptr = &ptr[0x18];

	for (i = 0; i < dir_count; i++) {
		/* name size */
		name_size = *(uint64_t *)(&ptr[0x00]);
		if (name_size != 4) {
			ptr = skip_dir(ptr);
			continue;
		}

		if ((ptr[0x18] != 'b')
		 || (ptr[0x19] != 'o')
		 || (ptr[0x1a] != 'o')
		 || (ptr[0x1b] != 't')) {
			ptr = skip_dir(ptr);
			continue;
		}

		/* found '/boot' */
		break;
	}

	if (i >= dir_count) {
		/* TODO : error message: '/boot' not found. */
		return false;
	}

	/* get the number of subdirectories in
	 * 'boot' */
	dir_count = *(uint64_t *) &ptr[0x08];

	/* We have to skip by the
	 * the subdirectories to get to the
	 * file array, where we'll find the
	 * 'kernel' file. */
	file_count = *(uint64_t *) &ptr[0x10];

	/* point to the subdirectories of '/boot'. */
	ptr = &ptr[0x18 + name_size];

	for (i = 0; i < dir_count; i++)
		ptr = skip_dir(ptr);

	for (i = 0; i < file_count; i++) {

		name_size = *(uint64_t *) &ptr[0x00];
		if (name_size != (sizeof("kernel") - 1)) {
			ptr = skip_file(ptr);
			continue;
		}

		if ((ptr[0x10] != 'k')
		 || (ptr[0x11] != 'e')
		 || (ptr[0x12] != 'r')
		 || (ptr[0x13] != 'n')
		 || (ptr[0x14] != 'e')
		 || (ptr[0x15] != 'l')) {
			ptr = skip_file(ptr);
			continue;
		}

		/* found the kernel */
		kernel->name_size = *(uint64_t *) &ptr[0x00];
		kernel->data_size = *(uint64_t *) &ptr[0x08];
		kernel->name = &ptr[0x10];
		kernel->data = &ptr[0x10 + name_size];
		return true;
	}

	/* TODO : error message: 'kernel' not found in '/boot' */

	return false;
}
