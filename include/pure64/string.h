/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_STRING_H
#define PURE64_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

void pure64_memset(void *dst, int value, unsigned long int size);

void pure64_memcpy(void *dst, const void *src, unsigned long int size);

unsigned long int pure64_strlen(const char *src);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_STRING_H */
