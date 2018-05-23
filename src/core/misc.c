/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "misc.h"

#include <pure64/core/stream.h>

int encode_uint16(pure64_uint16 n, struct pure64_stream *file) {

	int err;
	unsigned char buf[2];

	buf[0] = (n >> 0) & 0xff;
	buf[1] = (n >> 8) & 0xff;

	err = pure64_stream_write(file, buf, 2);
	if (err != 0)
		return err;

	return 0;
}

int encode_uint32(pure64_uint32 n, struct pure64_stream *file) {

	unsigned char buf[4];
	buf[0] = (n >> 0) & 0xff;
	buf[1] = (n >> 8) & 0xff;
	buf[2] = (n >> 16) & 0xff;
	buf[3] = (n >> 24) & 0xff;

	int err = pure64_stream_write(file, buf, 4);
	if (err != 0)
		return err;

	return 0;
}

int encode_uint64(pure64_uint64 n, struct pure64_stream *file) {

	int err;
	unsigned char buf[8];

	buf[0] = (n >> 0x00) & 0xff;
	buf[1] = (n >> 0x08) & 0xff;
	buf[2] = (n >> 0x10) & 0xff;
	buf[3] = (n >> 0x18) & 0xff;
	buf[4] = (n >> 0x20) & 0xff;
	buf[5] = (n >> 0x28) & 0xff;
	buf[6] = (n >> 0x30) & 0xff;
	buf[7] = (n >> 0x38) & 0xff;

	err = pure64_stream_write(file, buf, 8);
	if (err != 0)
		return err;

	return 0;
}

int decode_uint16(pure64_uint16 *n_ptr, struct pure64_stream *file) {

	unsigned char buf[2];

	int err = pure64_stream_read(file, buf, 2);
	if (err != 0)
		return err;

	pure64_uint16 n = 0;
	n |= ((pure64_uint16) buf[0]) << 0;
	n |= ((pure64_uint16) buf[1]) << 8;

	*n_ptr = n;

	return 0;
}

int decode_uint32(pure64_uint32 *n_ptr, struct pure64_stream *file) {

	unsigned char buf[4];

	int err = pure64_stream_read(file, buf, sizeof(buf));
	if (err != 0)
		return err;

	pure64_uint32 n = 0;
	n |= ((pure64_uint32) buf[0]) << 0;
	n |= ((pure64_uint32) buf[1]) << 8;
	n |= ((pure64_uint32) buf[2]) << 16;
	n |= ((pure64_uint32) buf[3]) << 24;

	*n_ptr = n;

	return 0;
}

int decode_uint64(pure64_uint64 *n_ptr, struct pure64_stream *file) {

	int err;
	unsigned char buf[8];

	err = pure64_stream_read(file, buf, 8);
	if (err != 0)
		return err;

	pure64_uint64 n = 0;
	n |= ((pure64_uint64) buf[0] << 0x00);
	n |= ((pure64_uint64) buf[1] << 0x08);
	n |= ((pure64_uint64) buf[2] << 0x10);
	n |= ((pure64_uint64) buf[3] << 0x18);
	n |= ((pure64_uint64) buf[4] << 0x20);
	n |= ((pure64_uint64) buf[5] << 0x28);
	n |= ((pure64_uint64) buf[6] << 0x30);
	n |= ((pure64_uint64) buf[7] << 0x38);

	*n_ptr = n;

	return 0;
}
