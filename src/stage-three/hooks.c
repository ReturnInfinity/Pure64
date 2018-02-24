/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "hooks.h"

#include "map.h"

#include <pure64/memory.h>

#include <stdint.h>

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

struct pure64_map *hooks_map = NULL;

void pure64_init_memory_hooks(struct pure64_map *map) {
	hooks_map = map;
}

void *pure64_malloc(uint64_t size) {
	if (hooks_map == NULL)
		return NULL;
	else
		return pure64_map_malloc(hooks_map, size);
}

void *pure64_realloc(void *addr, uint64_t size) {
	if (hooks_map == NULL)
		return NULL;
	else
		return pure64_map_realloc(hooks_map, addr, size);
}

void pure64_free(void *addr) {
	if (hooks_map != NULL)
		pure64_map_free(hooks_map, addr);
}
