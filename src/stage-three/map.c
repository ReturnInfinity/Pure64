/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "map.h"

#include "alloc.h"
#include "e820.h"

#include <pure64/error.h>
#include <pure64/string.h>

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

#define BOUNDARY 0x1000

#if defined (__x86_64)
#define FREE_START 0x70000
#elif defined (__riscv)
#define FREE_START 0x81000000
#else
#error "Unknown architecture"
#endif

static struct pure64_alloc *append_alloc(struct pure64_map *map, struct pure64_alloc *alloc) {

	struct pure64_alloc *alloc_table;
	uint64_t alloc_table_size;

	alloc_table = map->alloc_table;

	alloc_table_size = map->alloc_count + 1;
	alloc_table_size *= sizeof(struct pure64_alloc);

	alloc_table = pure64_map_realloc(map, alloc_table, alloc_table_size);
	if (alloc_table == NULL)
		return NULL;

	map->alloc_table = alloc_table;
	map->alloc_count++;

	map->alloc_table[map->alloc_count - 1] = *alloc;

	return &map->alloc_table[map->alloc_count - 1];
}

static void sort_alloc_table(struct pure64_map *map) {

	uint64_t i;
	struct pure64_alloc tmp;
	struct pure64_alloc *a;
	struct pure64_alloc *b;
	uint64_t addr_a;
	uint64_t addr_b;
	int sorted_flag;

bubbleSortLoop:

	sorted_flag = 0;

	for (i = 0; (i < map->alloc_count) && ((i + 1) < map->alloc_count); i++) {
		a = &map->alloc_table[i];
		b = &map->alloc_table[i + 1];
		addr_a = (uint64_t) a->addr;
		addr_b = (uint64_t) b->addr;
		if (addr_a > addr_b) {
			pure64_memcpy(&tmp, a, sizeof(struct pure64_alloc));
			pure64_memcpy(a, b, sizeof(struct pure64_alloc));
			pure64_memcpy(b, a, sizeof(struct pure64_alloc));
			sorted_flag = 1;
		}
	}

	if (sorted_flag)
		goto bubbleSortLoop;
}

static void *find_suitable_addr(struct pure64_map *map, uint64_t size) {

	struct pure64_alloc *alloc;
	/* candidate address */
	uint64_t addr;
	/* e820 entry address
	 * and size */
	uint64_t addr2;
	uint64_t size2;
	/* allocation entry address
	 * and size */
	uint64_t addr3;
	uint64_t size3;
	uint64_t i;

	const struct pure64_e820 *e820 = map->e820;

	/* Initial address is one.
	 * It should not be zero,
	 * because too many functions
	 * use zero to check for allocation
	 * errors. */

	addr = 1;

	/* Search the E820 map for a free
	 * entry */

	while (!pure64_e820_end(e820)) {

		/* Make sure that the E820 entry
		 * points to usable memory. */
		if (!pure64_e820_usable(e820)) {
			e820 = pure64_e820_next(e820);
			continue;
		}

		addr2 = (uint64_t) e820->addr;
		size2 = (uint64_t) e820->size;

		/* Check if E820 section can
		 * fit the requested size. */
		if (size2 < size) {
			e820 = pure64_e820_next(e820);
			continue;
		}

		/* Ensure that the memory
		 * does not end up being
		 * allocated at address zero. */
		if (addr2 == 0)
			addr = 1;
		else
			addr = addr2;

		/* Iterate the allocation table
		 * and make sure that new address
		 * will not overlap with existing
		 * memory allocations. */

		for (i = 0; i < map->alloc_count; i++) {

			alloc = &map->alloc_table[i];
			addr3 = (uint64_t) alloc->addr;
			size3 = alloc->reserved;

			/* Check if this entry is in the
			 * usable memory section from the
			 * E820 entry. */
			if ((addr3 < addr2) || (addr3 >= (addr2 + size2))) {
				/* Not in this E820 section,
				 * we can skip it. */
				continue;
			}

			/* Check if candidate address
			 * overlaps this entry in the
			 * allocation table. */
			if ((addr + size) > addr3) {
				/* Move candidate address to the
				 * end of the existing allocation. */
				addr = addr3 + size3;
				/* Check to make sure that the candidate
				 * address still fits the requirement size. */
				if ((addr + size) >= (addr2 + size2)) {
					/* The candidate address can no longer
					 * fit the size requirement. */
					break;
				}
			}
		}

		/* Check if the loop exited early.
		 * If the loop exited early, that
		 * means that the E820 entry does
		 * not contain a suitable space for
		 * the memory block. */

		if (i < map->alloc_count) {
			/* Continue to the next
			 * E820 entry, and search
			 * the allocation table
			 * again. */
			e820 = pure64_e820_next(e820);
			continue;
		}

		/* If the loop has reached this point,
		 * that means that the candidate address
		 * will not overlap with an existing
		 * allocation. We may return it to the
		 * caller now. */

		return (void *) addr;
	}

	/* If this point has been reached,
	 * that means that none of the E820
	 * entries contain enough memory for
	 * this allocation. */

	return NULL;
}

static struct pure64_alloc *find_alloc_entry(struct pure64_map *map, void *addr) {

	uint64_t i;

	for (i = 0; i < map->alloc_count; i++) {
		if (map->alloc_table[i].addr == addr)
			return &map->alloc_table[i];
	}

	return NULL;
}

void pure64_map_init(struct pure64_map *map) {

	/* Initialize E820 address */

	const struct pure64_e820 *e820 = pure64_e820_get();

	map->e820 = e820;

	/* Find an address to start the
	 * allocation table at. It will
	 * move locations when it needs to.
	 */

	uint64_t addr = FREE_START;

	while (!pure64_e820_end(e820)) {

		/* Check that the memory is usable. */
		if (!pure64_e820_usable(e820)) {
			e820 = pure64_e820_next(e820);
			continue;
		}

		/* Check that the memory is not already
		 * used by Pure64. */
		addr = (uint64_t) e820->addr;

		uint64_t size = e820->size;

		if ((addr + size) < FREE_START) {
			e820 = pure64_e820_next(e820);
			continue;
		} else if (addr < FREE_START) {
			if ((FREE_START - addr) > size) {
				e820 = pure64_e820_next(e820);
				continue;
			}
			size -= FREE_START - addr;
			addr = FREE_START;
		}

		/* Check that the memory can hold at least
		 * two entries. One for the initial Pure64
		 * memory map, and the other for the allocation
		 * table itself. */
		if (size < (sizeof(struct pure64_alloc) * 2)) {
			e820 = pure64_e820_next(e820);
			continue;
		}

		/* Found a suitable location for the
		 * initial allocation table. */
		break;
	}

	/* Check if search was successful. */
	if (pure64_e820_end(e820)) {
		/* Search was not successful.
		 * Future calls for memory allocations
		 * will fail. */
		map->alloc_table = NULL;
		map->alloc_count = 0;
	} else {
		/* Search was a success.
		 * Initialize the memory allocation
		 * table at the new address. */
		map->alloc_table = (struct pure64_alloc *) addr;
		map->alloc_count = 2;
		/* Pure64 initial memory map */
		map->alloc_table[0].addr = (void *) 0x00;
		map->alloc_table[0].size = FREE_START;
		map->alloc_table[0].reserved = FREE_START;
		/* allocation table (self reference) */
		map->alloc_table[1].addr = map->alloc_table;
		map->alloc_table[1].size = sizeof(struct pure64_alloc) * 2;
		map->alloc_table[1].reserved = sizeof(struct pure64_alloc) * 2;
	}
}

int pure64_map_reserve(struct pure64_map *map, void *ptr, uint64_t size) {

	/* Address 'ptr' as a 64-bit
	 * unsigned integer. */
	uint64_t addr;
	/* Either E820 address or
	 * allocation address */
	uint64_t addr2;

	addr = (uint64_t) ptr;

	const struct pure64_e820 *e820 = map->e820;

	/* Search the E820 entries and make
	 * sure that the memory is usable
	 * in this region. */

	while (pure64_e820_end(e820)) {

		/* Ensure that the region is usable. */
		if (!pure64_e820_usable(e820)) {
			e820 = pure64_e820_next(e820);
			continue;
		}

		/* Check if the address is contained
		 * by the E820 region. */
		addr2 = (uint64_t) e820->addr;
		if (addr < addr2)
			return PURE64_ENOMEM;
		else if ((addr + size) > (addr2 + e820->size))
			return PURE64_ENOMEM;
		else
			break;

		/* It is not, so continue
		 * and check the next record. */
		e820 = pure64_e820_next(e820);
	}

	/* If the loop ended early, that
	 * means that the memory region was
	 * found. */

	if (pure64_e820_end(e820)) {
		return PURE64_ENOMEM;
	}

	/* Search the allocation entries to
	 * see if any of them might overlap. */

	return 0;
}

void *pure64_map_malloc(struct pure64_map *map, uint64_t size) {

	struct pure64_alloc alloc;
	struct pure64_alloc *alloc_ent;

	if (map->alloc_table == NULL)
		return NULL;

	alloc.size = size;

	if ((size % BOUNDARY) != 0)
		size += BOUNDARY - (size % BOUNDARY);

	alloc.reserved = size;
	alloc.addr = NULL;

	/** The allocation entry must be allocated
	 * first, so that the new space occupied by
	 * the allocation table is known before finding
	 * an address for this allocation. */

	alloc_ent = append_alloc(map, &alloc);
	if (alloc_ent == NULL)
		return NULL;

	/* Do not allow for the allocation we just
	 * added to be visible to the find_suitable_addr
	 * function. */

	map->alloc_count--;

	alloc_ent->addr = find_suitable_addr(map, size);
	if (alloc_ent->addr == NULL)
		return NULL;

	/* Allow for the last allocation to be visible again.
	 * */

	map->alloc_count++;

	/* Sort allocation table, in
	 * case the address that was
	 * found is not the highest
	 * address in the allocation
	 * table. */

	sort_alloc_table(map);

	return alloc_ent->addr;
}

void *pure64_map_realloc(struct pure64_map *map, void *addr, uint64_t size) {

	/* Existing allocation table
	 * entry. */
	struct pure64_alloc *alloc;
	/* The next address for the
	 * memory block. */
	void *addr2;
	/* The number of bytes reserved
	 * for a new allocation (if one
	 * is actually made). */
	uint64_t reserved;

	/* Check if caller passed NULL,
	 * which means they want a completely
	 * new memory section. */

	if (addr == NULL) {
		return pure64_map_malloc(map, size);
	}

	/* Find the entry in the allocation
	 * table, so we know how much data
	 * to copy over. */

	alloc = find_alloc_entry(map, addr);
	if (alloc == NULL)
		return NULL;

	/* Check to make sure that there is
	 * more memory already reserved for
	 * this entry. */

	if (alloc->reserved >= size) {
		/* The allocation entry
		 * has enough memory reserved
		 * already. */
		alloc->size = size;
		return addr;
	}

	/* Give the size of the allocation
	 * room to grow. Allocate it on a
	 * boundary. */

	reserved = size;

	if ((reserved % BOUNDARY) != 0) {
		reserved += BOUNDARY - (reserved % BOUNDARY);
	}

	/* Find an address to put the new data
	 * into. */

	addr2 = find_suitable_addr(map, size);
	if (addr2 == NULL)
		return NULL;

	/* Assign the new address to the allocation
	 * table entry and resort the entries. */

	alloc->addr = addr2;
	alloc->size = size;
	alloc->reserved = reserved;

	/* Copy memory from old location to the
	 * new location. */

	pure64_memcpy(addr2, addr, alloc->size);

	sort_alloc_table(map);

	return (void *) addr2;
}

void pure64_map_free(struct pure64_map *map, void *addr) {

	struct pure64_alloc *alloc;

	/* Check if the address is a null
	 * pointer. This means that this
	 * function should do nothing. */
	if (addr == NULL)
		return;

	/* This function finds the allocation
	 * entry and puts it at the end of the
	 * table. Once it's at the end of the
	 * table, it decrements the entry count
	 * so that the entry is no longer visible. */

	/* Find the allocation entry. */
	alloc = find_alloc_entry(map, addr);
	if (alloc == NULL)
		return;

	/* This will cause the sort function to put
	 * the entry at the end of the table. */
	alloc->addr = (void *) 0xffffffffffffffff;
	sort_alloc_table(map);

	map->alloc_count--;
}
