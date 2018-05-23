/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_DEBUG_H
#define PURE64_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void debug(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_DEBUG_H */
