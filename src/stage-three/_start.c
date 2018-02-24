/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dir.h>
#include <pure64/error.h>
#include <pure64/file.h>
#include <pure64/fs.h>
#include <pure64/string.h>

#include "ahci.h"
#include "alloc.h"
#include "debug.h"
#include "e820.h"
#include "hooks.h"
#include "map.h"
#include "string.h"

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

typedef void (*kernel_entry)(void);

static int find_file_system(struct pure64_map *map);

static int load_kernel(struct pure64_map *map,
                       struct pure64_file *kernel);

void _start(void) __attribute((section(".text._start")));

void _start(void) {

	struct pure64_map map;

	pure64_map_init(&map);

	pure64_init_memory_hooks(&map);

	debug("Searching for file system.\n");

	find_file_system(&map);
}

static int ahci_visit_port(void *map_ptr, volatile struct ahci_port *port) {

	int err;
	struct pure64_fs fs;
	struct pure64_file *kernel;
	struct ahci_stream stream;

	/* Bail out if port isn't SATA */
	if (!ahci_port_is_sata_drive(port))
		return 0;

	/* Initialize the port as a stream. */
	ahci_stream_init(&stream, port);

	/* Set the stream to the correct position. */
	pure64_stream_set_pos(&stream.base, PURE64_FS_SECTOR * 512);

	/* Initialize the file system. */
	pure64_fs_init(&fs);

	/* Import the file system from the
	 * AHCI stream. */
	err = pure64_fs_import(&fs, &stream.base);
	if (err != 0) {
		if (err == PURE64_EINVAL)
			debug("Failed to import FS: Invalid file system signature.\n");
		else
			debug("Failed to import FS: %s\n", pure64_strerror(err));
		pure64_fs_free(&fs);
		return 0;
	}

	debug("Found file system.\n");

	kernel = pure64_fs_open_file(&fs, "/boot/kernel");
	if (kernel == NULL) {
		debug("Failed to open kernel.\n");
		debug("Ensure that '/boot/kernel' exists.\n");
		pure64_fs_free(&fs);
		return 0;
	}

	debug("Loading kernel.\n");

	load_kernel((struct pure64_map *) map_ptr, kernel);

	debug("Kernel exited.\n");

	pure64_fs_free(&fs);

	/* One means stop the AHCI visitor */

	return 1;
}

static int find_file_system(struct pure64_map *map) {

	struct ahci_visitor visitor;

	visitor.data = map;
	visitor.visit_base = NULL;
	visitor.visit_port = ahci_visit_port;

	ahci_visit(&visitor);

	return 0;
}

static void load_failure(const char *msg) {
	debug("Failed to load kernel: \"%s\"\n", msg);
}

static int load_kernel_elf(struct pure64_map *map,
                           struct pure64_file *kernel) {

	const unsigned char *data;
	uint64_t data_size;
	uint16_t i = 0;

	data = (const unsigned char *) kernel->data;

	data_size = kernel->data_size;

	/* check that the entire header is there */
	if (data_size < 0x3E)
		return PURE64_EINVAL;

	/* verify it is 64-bit */
	if (data[0x04] != 2) {
		load_failure("Kernel is not 64-bit.");
		return PURE64_EINVAL;
	}

	/* verify is little-endian */
	if (data[0x05] != 1) {
		load_failure("Kernel is not little endian.");
		return PURE64_EINVAL;
	}

	/* verify is executable */
	if ((data[0x10] != 0x02)
	 || (data[0x11] != 0x00)) {
		load_failure("Kernel is not an executable.");
		return PURE64_EINVAL;
	}

	/* TODO : what is this for? */
	if ((data[0x12] != 0x3e)
	 || (data[0x13] != 0x00))
		return PURE64_EINVAL;

	kernel_entry kentry = (kernel_entry) *(uint64_t *) &data[0x18];

	uint64_t e_phoff = *(uint64_t *) &data[0x20];

	uint16_t e_phentsize = *(uint16_t *) &data[0x36];

	uint16_t e_phnum = *(uint16_t *) &data[0x38];

	/* check to make sure the all the program
	 * headers are available in memory */
	if (data_size < (e_phoff + (e_phnum * e_phentsize))) {
		load_failure("Kernel file is corrupt.");
		return PURE64_EINVAL;
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
		if (p_filesz > p_memsz) {
			load_failure("Kernel file is corrupt.");
			return PURE64_EINVAL;
		}

		/* Pure64 currently only supports
		 * loading at or above this address */
		if (vaddr < ((void *) 0x100000)) {
			load_failure("Invalid load address.");
			return PURE64_EINVAL;
		}

		/* Ensure that the address is available
		 * and reserve it so that memory can't
		 * be allocated there. */
		if (pure64_map_reserve(map, vaddr, p_filesz) != 0) {
			load_failure("Failed to reserve kernel memory.");
			return PURE64_ENOMEM;
		}

		/* Copy code over to address */
		pure64_memcpy(vaddr, &data[p_offset], p_filesz);
	}

	/* Call the kernel entry point.  */

	kentry();

	return 0;
}

static int load_kernel_bin(struct pure64_map *map,
                           struct pure64_file *kernel) {

	/* Flat binary kernels are loaded
	 * into the 1 MiB address. */

	int err = pure64_map_reserve(map, (void *) 0x1000000, kernel->data_size);
	if (err != 0)
		return err;

	/* Get the entry point address. */

	kernel_entry kentry = (kernel_entry) kernel->data;

	/* Call the entry point.
	 * Hope that it works.
	 * */

	kentry();

	return 0;
}

static int load_kernel(struct pure64_map *map,
                       struct pure64_file *kernel) {

	const unsigned char *data;
	uint64_t data_size;

	data = (const unsigned char *) kernel->data;

	data_size = kernel->data_size;

	/* Check if the kernel is in ELF format. */
	if ((data_size >= 4)
	 && (data[0x00] == 0x7f)
	 && (data[0x01] == 'E')
	 && (data[0x02] == 'L')
	 && (data[0x03] == 'F')) {
		/* Found the ELF signature. */
		return load_kernel_elf(map, kernel);
	}

	/* TODO : check for PE */

	/* Kernel is probably a flat binary. */

	return load_kernel_bin(map, kernel);
}
