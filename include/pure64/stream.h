/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file */

#ifndef PURE64_STREAM_H
#define PURE64_STREAM_H

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
	int (*get_size)(void *data, uint64_t *size);
	/** Get position callback */
	int (*get_pos)(void *data, uint64_t *offset);
	/** Read callback */
	int (*read)(void *data, void *buf, uint64_t buf_size);
	/** Set position callback */
	int (*set_pos)(void *data, uint64_t offset);
	/** Write callback */
	int (*write)(void *data, const void *buf, uint64_t buf_size);
};

/** Initializes the stream for use.
 * @param stream An uninitialized stream structure.
 * */

void pure64_stream_init(struct pure64_stream *stream);

/** Get the size of the stream. If
 * the stream is resizeable, then it is
 * the span of the stream that actually
 * contains data.
 * @param stream An initialized stream structure.
 * @param size The pointer to the variable that
 * will receive the size of the stream.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_stream_get_size(struct pure64_stream *stream, uint64_t *size);

/** Gets the current position of the stream.
 * @param stream An initialized stream structure.
 * @param pos A pointer to the variable that will
 * receive the current position of the stream.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_stream_get_pos(struct pure64_stream *stream, uint64_t *pos);

/** Reads data from the stream.
 * @param stream An initialized stream structure.
 * @param buf The buffer to put the data in.
 * @param buf_size The number of bytes to read and
 * the size of the buffer.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_stream_read(struct pure64_stream *stream, void *buf, uint64_t buf_size);

/** Sets the position of the stream.
 * @param stream An initialized stream structure.
 * @param pos The position to set the stream to.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_stream_set_pos(struct pure64_stream *stream, uint64_t pos);

/** Writes data to the stream.
 * @param stream An initialized stream structure.
 * @param buf The buffer containing the data.
 * @param buf_size The number of bytes in the buffer.
 * @returns Zero on success, non-zero on failure.
 * */

int pure64_stream_write(struct pure64_stream *stream, const void *buf, uint64_t buf_size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_STREAM_H */
