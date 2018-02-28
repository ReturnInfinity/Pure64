/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "misc.h"

#include <pure64/stream.h>

int encode_uint16(uint16_t n, struct pure64_stream *file) {

	int err;
	unsigned char buf[2];

	buf[0] = (n >> 0) & 0xff;
	buf[1] = (n >> 8) & 0xff;

	err = pure64_stream_write(file, buf, 2);
	if (err != 0)
		return err;

	return 0;
}

int encode_uint32(uint32_t n, struct pure64_stream *file) {

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

int encode_uint64(uint64_t n, struct pure64_stream *file) {

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

int decode_uint16(uint16_t *n_ptr, struct pure64_stream *file) {

	unsigned char buf[2];

	int err = pure64_stream_read(file, buf, 2);
	if (err != 0)
		return err;

	uint16_t n = 0;
	n |= ((uint16_t) buf[0]) << 0;
	n |= ((uint16_t) buf[1]) << 8;

	*n_ptr = n;

	return 0;
}

int decode_uint64(uint64_t *n_ptr, struct pure64_stream *file) {

	int err;
	unsigned char buf[8];

	err = pure64_stream_read(file, buf, 8);
	if (err != 0)
		return err;

	uint64_t n = 0;
	n |= ((uint64_t) buf[0] << 0x00);
	n |= ((uint64_t) buf[1] << 0x08);
	n |= ((uint64_t) buf[2] << 0x10);
	n |= ((uint64_t) buf[3] << 0x18);
	n |= ((uint64_t) buf[4] << 0x20);
	n |= ((uint64_t) buf[5] << 0x28);
	n |= ((uint64_t) buf[6] << 0x30);
	n |= ((uint64_t) buf[7] << 0x38);

	*n_ptr = n;

	return 0;
}
