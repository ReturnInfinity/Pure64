/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_HOOKS_H
#define PURE64_HOOKS_H

#ifdef __cplusplus
extern "C" {
#endif

struct pure64_map;

void pure64_init_memory_hooks(struct pure64_map *map);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_HOOKS_H */
