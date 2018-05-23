/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2018 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "ahci.h"

int ahci_visit(struct ahci_visitor *visitor) {
	(void) visitor;
	return 0;
}

void ahci_stream_init(struct ahci_stream *stream, volatile struct ahci_port *port) {
	pure64_stream_init(&stream->base);
	stream->base.data = stream;
	stream->base.read = NULL;
	stream->base.get_pos = NULL;
	stream->base.set_pos = NULL;
	stream->port = port;
	stream->position = 0;
	stream->buf = NULL;
	stream->buf_sector = 0;
	stream->buf_sector_count = 0;
}
