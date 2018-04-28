/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/core/stream.h>

#include <pure64/core/error.h>

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

void pure64_stream_init(struct pure64_stream *stream) {
	stream->data = NULL;
	stream->get_size = NULL;
	stream->get_pos = NULL;
	stream->read = NULL;
	stream->set_pos = NULL;
	stream->write = NULL;
}

int pure64_stream_get_size(struct pure64_stream *stream, pure64_uint64 *size) {
	if (stream->get_size != NULL)
		return stream->get_size(stream->data, size);
	else
		return PURE64_ENOSYS;
}

int pure64_stream_get_pos(struct pure64_stream *stream, pure64_uint64 *pos) {
	if (stream->get_pos != NULL)
		return stream->get_pos(stream->data, pos);
	else
		return PURE64_ENOSYS;
}

int pure64_stream_read(struct pure64_stream *stream, void *buf, pure64_uint64 buf_size) {
	if (stream->read != NULL)
		return stream->read(stream->data, buf, buf_size);
	else
		return PURE64_ENOSYS;
}

int pure64_stream_set_pos(struct pure64_stream *stream, pure64_uint64 pos) {
	if (stream->set_pos != NULL)
		return stream->set_pos(stream->data, pos);
	else
		return PURE64_ENOSYS;
}

int pure64_stream_write(struct pure64_stream *stream, const void *buf, pure64_uint64 buf_size) {
	if (stream->write != NULL)
		return stream->write(stream->data, buf, buf_size);
	else
		return PURE64_ENOSYS;
}
