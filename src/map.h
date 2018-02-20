/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_MAP_H
#define PURE64_MAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ahci_driver;
struct pure64_alloc;
struct pure64_e820;
struct pure64_info;

/** The memory map that Pure64
 * configures for the kernel.
 * */

struct pure64_map {
	/** The AHCI driver structure. */
	struct ahci_driver *ahci_driver;
	/** The memory allocation table. */
	struct pure64_alloc *alloc_table;
	/** The number of entries in the
	 * allocation table. */
	uint64_t alloc_count;
	/** Info related to the
	 * host machine. */
	struct pure64_info *info;
	/** The base address for the
	 * E820 memory entries. */
	struct pure64_e820 *e820;
};

/** Initializes a Pure64 map.
 * This function simply sets the
 * appropriate addresses of the
 * memory map structure. It does
 * not interface with firmware.
 * */

void pure64_map_init(struct pure64_map *map);

/** Allocate a block of memory
 * for general purpose usage.
 * @param map An initialized memory map.
 * @param size The number of bytes to allocate.
 * @returns An address containing a block
 * of memory for general purpose usage.
 * */

void *pure64_map_malloc(struct pure64_map *map,
                        uint64_t size);

/** Resize an existing portion of memory.
 * @param map An initialized memory map.
 * @param addr The address of the existing
 * memory block. If this parameter is NULL,
 * then a new memory block is allocated.
 * @param size The number of bytes that the
 * new section should fit.
 * @returns The new address of the memory
 * block.
 * */

void *pure64_map_realloc(struct pure64_map *map,
                         void *addr,
                         uint64_t size);

/** Reserve a specific section of memory for use.
 * @param map An initialized memory map.
 * @param addr The address of the memory section.
 * @param size The number of bytes to reserve for
 * the section.
 * */

void *pure64_map_reserve(struct pure64_map *map,
                         void *addr,
                         uint64_t size);

/** Release a block of memory, so
 * that it may be used by another
 * function.
 * @param map An initialzied memory map.
 * @param addr The address that was returned
 * by a call to @pref pure64_map_alloc.
 * */

void pure64_map_free(struct pure64_map *map,
                     void *addr);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_MAP_H */
