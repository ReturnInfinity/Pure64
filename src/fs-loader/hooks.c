/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "hooks.h"

#include "map.h"

#include <pure64/core/memory.h>
#include <pure64/core/types.h>

struct pure64_map *hooks_map = pure64_null;

void pure64_init_memory_hooks(struct pure64_map *map) {
	hooks_map = map;
}

void *pure64_malloc(pure64_uint64 size) {
	if (hooks_map == pure64_null)
		return pure64_null;
	else
		return pure64_map_malloc(hooks_map, size);
}

void *pure64_realloc(void *addr, pure64_uint64 size) {
	if (hooks_map == pure64_null)
		return pure64_null;
	else
		return pure64_map_realloc(hooks_map, addr, size);
}

void pure64_free(void *addr) {
	if (hooks_map != pure64_null)
		pure64_map_free(hooks_map, addr);
}
