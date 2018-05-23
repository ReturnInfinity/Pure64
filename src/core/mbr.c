/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/core/mbr.h>

#include <pure64/core/error.h>
#include <pure64/core/string.h>
#include <pure64/core/stream.h>

#include "misc.h"

void pure64_mbr_zero(struct pure64_mbr *mbr) {
	pure64_memset(mbr, 0, sizeof(*mbr));
}

int pure64_mbr_check(const struct pure64_mbr *mbr) {

	if (mbr->boot_signature != 0xaa55)
		return PURE64_EINVAL;

	return 0;
}

int pure64_mbr_read(struct pure64_mbr *mbr, struct pure64_stream *stream) {

	int err;

	err = pure64_stream_set_pos(stream, 0x00);
	if (err != 0)
		return err;

	err = pure64_stream_read(stream, mbr->code, sizeof(mbr->code));
	if (err != 0)
		return err;

	err = pure64_dap_read(&mbr->st2dap, stream);
	if (err != 0)
		return err;

	err = pure64_dap_read(&mbr->st3dap, stream);
	if (err != 0)
		return err;

	err = decode_uint16(&mbr->boot_signature, stream);
	if (err != 0)
		return err;

	return 0;
}

int pure64_mbr_write(const struct pure64_mbr *mbr, struct pure64_stream *stream) {

	int err;

	err = pure64_stream_set_pos(stream, 0x00);
	if (err != 0)
		return err;

	err = pure64_stream_write(stream, mbr->code, sizeof(mbr->code));
	if (err != 0)
		return err;

	err = pure64_dap_write(&mbr->st2dap, stream);
	if (err != 0)
		return err;

	err = pure64_dap_write(&mbr->st3dap, stream);
	if (err != 0)
		return err;

	err = encode_uint16(mbr->boot_signature, stream);
	if (err != 0)
		return err;

	return 0;
}
