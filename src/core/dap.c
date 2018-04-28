/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dap.h>

#include <pure64/stream.h>

#include "misc.h"

int pure64_dap_read(struct pure64_dap *dap, struct pure64_stream *stream) {

	int err = pure64_stream_read(stream, dap->reserved, sizeof(dap->reserved));
	if (err != 0)
		return err;

	err = decode_uint16(&dap->sector_count, stream);
	if (err != 0)
		return err;

	err = decode_uint16(&dap->address, stream);
	if (err != 0)
		return err;

	err = decode_uint16(&dap->segment, stream);
	if (err != 0)
		return err;

	err = decode_uint64(&dap->sector, stream);
	if (err != 0)
		return err;

	return 0;
}

int pure64_dap_write(const struct pure64_dap *dap, struct pure64_stream *stream) {

	int err = pure64_stream_write(stream, dap->reserved, sizeof(dap->reserved));
	if (err != 0)
		return err;

	err = encode_uint16(dap->sector_count, stream);
	if (err != 0)
		return err;

	err = encode_uint16(dap->address, stream);
	if (err != 0)
		return err;

	err = encode_uint16(dap->segment, stream);
	if (err != 0)
		return err;

	err = encode_uint64(dap->sector, stream);
	if (err != 0)
		return err;

	return 0;
}
