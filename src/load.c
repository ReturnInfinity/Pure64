/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dir.h>
#include <pure64/file.h>

typedef void (*kernel_entry)(void);

static void pure64_memcpy(void *dst, const void *src, uint64_t size) {

	uint64_t i = 0;
	unsigned char *dst8 = (unsigned char *) dst;

	const unsigned char *src8 = (const unsigned char *) src;

	for (i = 0; i < size; i++)
		dst8[i] = src8[i];
}

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

	const unsigned char *data;
	uint64_t data_size;
	uint16_t i = 0;

	data = (const unsigned char *) kernel->data;

	data_size = kernel->data_size;

	/* check that the entire header is there */
	if (data_size < 0x3E)
		return;

	/* verify file signature */
	if ((data[0x00] != 0x7f)
	 || (data[0x01] != 'E')
	 || (data[0x02] != 'L')
	 || (data[0x03] != 'F')) {
		return;
	}

	/* verify it is 64-bit */
	if (data[0x04] != 2)
		return;

	/* verify is little-endian */
	if (data[0x05] != 1)
		return;

	/* verify is executable */
	if ((data[0x10] != 0x02)
	 || (data[0x11] != 0x00))
		return;

	if ((data[0x12] != 0x3e)
	 || (data[0x13] != 0x00))
		return;

	kernel_entry e_entry = (kernel_entry) *(uint64_t *) &data[0x18];

	uint64_t e_phoff = *(uint64_t *) &data[0x20];

	uint16_t e_phentsize = *(uint16_t *) &data[0x36];

	uint16_t e_phnum = *(uint16_t *) &data[0x38];

	/* check to make sure the all the program
	 * headers are available in memory */
	if (data_size < (e_phoff + (e_phnum * e_phentsize))) {
		return;
	}

	for (i = 0; i < e_phnum; i++) {

		unsigned char *ph = (unsigned char *) &data[e_phoff + (i * e_phentsize)];

		/* verify it's a loadable segment */
		if ((ph[0] != 0x01)
		 || (ph[1] != 0x00)
		 || (ph[2] != 0x00)
		 || (ph[3] != 0x00)) {
			continue;
		}

		/* offset of segment within the file */
		uint64_t p_offset = *(uint64_t *) &ph[0x08];

		/* virtual address of segment */
		void *vaddr = (void *) *(uint64_t *) &ph[0x10];

		/* size of segment on file */
		uint64_t p_filesz = *(uint64_t *) &ph[0x20];

		/* size of segment in memory */
		uint64_t p_memsz = *(uint64_t *) &ph[0x28];

		/* Verify the size of file is the same
		 * or smaller than the required area in
		 * memory */
		if (p_filesz > p_memsz)
			return;

		/* Pure64 currently only supports
		 * loading at this address. */
		if (vaddr != ((void *) 0x400000))
			return;

		/* TODO : properly allocate memory,
		 * checking to see if the address is
		 * available or not */

		/* Copy code over to address */
		pure64_memcpy(vaddr, &data[p_offset], p_filesz);

		/* Call the kernel */
		e_entry();

		/* If the kernel returns, exit
		 * this function */
		return;
	}
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

	uint64_t i = 0;

	ptr = (unsigned char *) &ptr64[3];

	ptr = &ptr[name_size];

	for (i = 0; i < dir_count; i++)
		ptr = skip_dir(ptr);
	
	for (i = 0; i < file_count; i++)
		ptr = skip_file(ptr);

	return ptr;
}

static bool find_kernel(struct pure64_file *kernel) {

	/* The kernel resides in '/boot/kernel'.
	 * Find the 'boot' directory, then look
	 * for the 'kernel' file.
	 * */

	/* beginning of the directories in '/' */
	unsigned char *ptr = (unsigned char *) 0xa000 + 16;

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
		kernel->name = (char *) &ptr[0x10];
		kernel->data = (void *) &ptr[0x10 + name_size];
		return true;
	}

	/* TODO : error message: 'kernel' not found in '/boot' */

	return false;
}
