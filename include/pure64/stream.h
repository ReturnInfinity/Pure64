/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file */

#ifndef PURE64_STREAM_H
#define PURE64_STREAM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A Pure64 stream.
 * */

struct pure64_stream {
	/** Implementation data */
	void *data;
	/** Get size callback */
	bool (*get_size)(void *data, uint64_t *size);
	/** Get position callback */
	bool (*get_pos)(void *data, uint64_t *offset);
	/** Read callback */
	bool (*read)(void *data, void *buf, uint64_t buf_size);
	/** Set position callback */
	bool (*set_pos)(void *data, uint64_t offset);
	/** Write callback */
	bool (*write)(void *data, const void *buf, uint64_t buf_size);
};

/** Initializes the stream for use.
 * @param stream An uninitialized stream structure.
 * */

void pure64_stream_init(struct pure64_stream *stream);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_STREAM_H */
