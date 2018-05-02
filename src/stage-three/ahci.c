/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "ahci.h"

#include "pci.h"

#include <pure64/core/error.h>
#include <pure64/core/string.h>
#include <pure64/core/memory.h>

#include "debug.h"

/* * * * * *
 * Constants
 * * * * * */

#ifndef NULL
#define NULL ((void *) 0x00)
#endif

#ifndef AHCI_PORT_CMD_CR
#define AHCI_PORT_CMD_CR (1 << 15)
#endif

#ifndef AHCI_PORT_CMD_FR
#define AHCI_PORT_CMD_FR (1 << 14)
#endif

#ifndef PRDT_COUNT
#define PRDT_COUNT 0x10
#endif

#ifndef TASK_FILE_ERROR
#define TASK_FILE_ERROR (1 << 30)
#endif

#ifndef ATA_DEV_BUSY
#define ATA_DEV_BUSY 0x80
#endif

#ifndef ATA_DEV_DRQ
#define ATA_DEV_DRQ 0x08
#endif

#ifndef AHCI_CACHE_MAX
#define AHCI_CACHE_MAX (4 * 1024 * 1024)
#endif

/* * * * * * * * * * * * * * *
 * AHCI Structure Declarations
 * * * * * * * * * * * * * * */

struct reg_d2h {
	/** Frame information structure type (0x5f) */
	pure64_uint8 fis_type;
	/** Port multiplier */
	pure64_uint8 port_multiplier:4;
	/** Reserved */
	pure64_uint8 reserved0:1;
	/** Data transfer direction. One
	 * means device to host and zero
	 * means host to device. */
	pure64_uint8 data_transfer_direction:1;
	/** Interrupt bit */
	pure64_uint8 interrupt_bit;
	/** Reserved */
	pure64_uint8 reserved1:1;
	/** Status register */
	pure64_uint8 status;
	/** Error register */
	pure64_uint8 error;
	/** LBA register 7:0 */
	pure64_uint8 lba0;
	/** LBA register 15:8 */
	pure64_uint8 lba1;
	/** LBA register 23:16 */
	pure64_uint8 lba2;
	/** Device register */
	pure64_uint8 device;
	/** LBA register 31:24 */
	pure64_uint8 lba3;
	/** LBA register 39:32 */
	pure64_uint8 lba4;
	/** LBA register 47:40 */
	pure64_uint8 lba5;
	/** Reserved */
	pure64_uint8 reserved2;
	/** Count register 7:0 */
	pure64_uint8 count_low;
	/** Count register 15:8 */
	pure64_uint8 count_high;
	/** Reserved */
	pure64_uint8 reserved3[6];
};

struct reg_h2d {
	/* FIS_TYPE_REG_H2D */
	pure64_uint8 fis_type;
	/* Port multiplier */
	pure64_uint8 pmport:4;
	/* Reserved */
	pure64_uint8 rsv0:3;
	/* 1: Command, 0: Control */
	pure64_uint8 c:1;
	/* Command register */
	pure64_uint8 command;
	/* Feature register, 7:0 */
	pure64_uint8 featurel;
	/* LBA low register, 7:0 */
	pure64_uint8 lba0;
	/* LBA mid register, 15:8 */
	pure64_uint8 lba1;
	/* LBA high register, 23:16 */
	pure64_uint8 lba2;
	/* Device register */
	pure64_uint8 device;
	/* LBA register, 31:24 */
	pure64_uint8 lba3;
	/* LBA register, 39:32 */
	pure64_uint8 lba4;
	/* LBA register, 47:40 */
	pure64_uint8 lba5;
	/* Feature register, 15:8 */
	pure64_uint8 featureh;
	/* Count register, 7:0 */
	pure64_uint8 countl;
	/* Count register, 15:8 */
	pure64_uint8 counth;
	/* Isochronous command completion */
	pure64_uint8 icc;
	/* Control register */
	pure64_uint8 control;
	/* Reserved */
	pure64_uint8 rsv1[4];
};

struct pio_setup {
	/** Frame information structure type (0x5f) */
	pure64_uint8 fis_type;
	/** Port multiplier */
	pure64_uint8 port_multiplier:4;
	/** Reserved */
	pure64_uint8 reserved0:1;
	/** Data transfer direction. One
	 * means device to host and zero
	 * means host to device. */
	pure64_uint8 data_transfer_direction:1;
	/** Interrupt bit */
	pure64_uint8 interrupt_bit;
	/** Reserved */
	pure64_uint8 reserved1:1;
	/** Status register */
	pure64_uint8 status;
	/** Error register */
	pure64_uint8 error;
	/** LBA register 7:0 */
	pure64_uint8 lba0;
	/** LBA register 15:8 */
	pure64_uint8 lba1;
	/** LBA register 23:16 */
	pure64_uint8 lba2;
	/** Device register */
	pure64_uint8 device;
	/** LBA register 31:24 */
	pure64_uint8 lba3;
	/** LBA register 39:32 */
	pure64_uint8 lba4;
	/** LBA register 47:40 */
	pure64_uint8 lba5;
	/** Reserved */
	pure64_uint8 reserved2;
	/** Count register 7:0 */
	pure64_uint8 count_low;
	/** Count register 15:8 */
	pure64_uint8 count_high;
	/** Reserved */
	pure64_uint8 reserved3;
	/** New value of status register */
	pure64_uint8 e_status;
	/** Transfer count register */
	pure64_uint16 transfer_count;
	/** Reserved */
	pure64_uint8 reserved4[2];
};

struct dma_setup {
	/** FIS type (0x41) */
	pure64_uint8 fis_type;
	/** Port multiplier */
	pure64_uint8 port_multiplier:4;
	/** Reserved */
	pure64_uint8 reserved0:1;
	/** Zero means host to device
	 * and one means device to host. */
	pure64_uint8 data_transfer_direction:1;
	/** Interrupt bit */
	pure64_uint8 interrupt_bit:1;
	/** Auto activate */
	pure64_uint8 activate:1;
	/** Reserved */
	pure64_uint8 reserved1[2];
	/** Direct memory access buffer identifier */
	pure64_uint64 dma_buffer_id;
	/** Reserved */
	pure64_uint8 reserved2[4];
	/** Byte offset into buffer. The first
	 * two bits must be zero. */
	pure64_uint32 dma_buffer_offset;
	/** Number of bytes to transfer. */
	pure64_uint32 transfer_count;
	/** Reserved */
	pure64_uint8 reserved3[4];
};

struct ahci_fis {
	/** Direct memory access setup FIS */
	struct dma_setup dsfis;
	/** Padding */
	pure64_uint8 pad0[4];
	/** PIO setup FIS */
	struct pio_setup psfs;
	/** Padding */
	pure64_uint8 pad1[12];
	/** D2H register fis */
	struct reg_d2h rfis;
	/** Padding */
	pure64_uint8 pad2[4];
	/** Set device bits FIS */
	pure64_uint64 sdbfis;
	/** */
	pure64_uint8 ufis[64];
	/** Reserved */
	pure64_uint8 reserved[0x100 - 0xa0];
};

struct command_header {
	/** Command FIS length, counted in 32-bit words */
	pure64_uint8 cfl:5;
	/** ATAPI */
	pure64_uint8 atapi:1;
	pure64_uint8 write:1;
	pure64_uint8 prefetchable:1;
	pure64_uint8 reset:1;
	pure64_uint8 bist:1;
	pure64_uint8 clear:1;
	pure64_uint8 reserved0:1;
	pure64_uint8 multiplier_port:1;
	/** Physical region descriptor table length,
	 * counted in entries */
	pure64_uint16 prdt_length;
	/** Physical region descriptor byte count
	 * transferred. */
	volatile pure64_uint32 prd_byte_count;
	/** Command table base address */
	struct ahci_addr command_table;
	/** Reserved */
	pure64_uint8 reserved1[16];
};

struct command_list {
	struct command_header headers[32];
};

struct prdt_entry {
	/** Data base address */
	struct ahci_addr data;
	/** Reserved */
	pure64_uint32 reserved0;
	/** Data byte count */
	pure64_uint32 byte_count:22;
	/** Reserved */
	pure64_uint32 reserved1:9;
	/** Interrupt on completion */
	pure64_uint32 i:1;
};

struct command_table {
	/** Command FIS */
	pure64_uint8 cfis[64];
	/** ATAPI command */
	pure64_uint8 acmd[16];
	/** Reserved */
	pure64_uint8 reserved[48];
	/** PRDT entries */
	struct prdt_entry entries[PRDT_COUNT];
};

/* * * * * * * * * * * * *
 * AHCI Address Functions
 * * * * * * * * * * * * */

void *ahci_addr_get(const volatile struct ahci_addr *addr) {

	pure64_uint64 ptr64;

	ptr64 = 0;
	ptr64 |= (((pure64_uint64) addr->upper) << 0x20) & 0xffffffff00000000;
	ptr64 |= (((pure64_uint64) addr->lower) << 0x00) & 0x00000000ffffffff;

	return (void *) ptr64;
}

void ahci_addr_set(volatile struct ahci_addr *addr, void *ptr) {

	pure64_uint64 ptr64;

	ptr64 = (pure64_uint64) ptr;

	addr->upper = (pure64_uint32) ((ptr64 >> 0x20) & 0xffffffff);
	addr->lower = (pure64_uint32) ((ptr64 >> 0x00) & 0xffffffff);
}

/* * * * * * * * * * *
 * AHCI Port Functions
 * * * * * * * * * * */

pure64_uint32 find_cmdslot(volatile struct ahci_port *port) {

	pure64_uint32 i;
	// If not set in SACT and CI, the slot is free
	pure64_uint32 slots = (port->sact | port->ci);

	for (i = 0; i < 32; i++) {

		if ((slots & 1) == 0) {
			return i;
		}

		slots >>= 1;
	}

	return i;
}

int ahci_port_is_sata_drive(const volatile struct ahci_port *port) {
	return port->sig == 0x101;
}

int ahci_port_read(volatile struct ahci_port *port,
                   pure64_uint64 sector,
                   pure64_uint32 sector_count,
                   void *buf) {

	pure64_uint32 spin;
	pure64_uint32 slot;
	pure64_uint64 byte_count;
	unsigned char *buf8;
	pure64_uint64 buf_offset;
	struct command_list *cmd_list;
	struct command_header *cmd_header;
	struct command_table *cmd_table;
	struct reg_h2d *cmd_fis;

	pure64_uint32 prdt_count;
	pure64_uint32 prdt_payload = 0x8000;
	pure64_uint32 i;

	port->is = ~0;

	slot = find_cmdslot(port);
	if (slot == 32) {
		return -1;
	}

	byte_count = sector_count * 512;

	/* Make sure that the read count does not
	 * exceed what's capable of the software. */
	if (byte_count > (prdt_payload * PRDT_COUNT)) {
		byte_count = prdt_payload * PRDT_COUNT;
		sector_count = byte_count / 512;
	}

	/* Find out how many PRDT entries are
	 * required for this operation. */
	if ((byte_count % prdt_payload) != 0)
		prdt_count = (byte_count + prdt_payload) / prdt_payload;
	else
		prdt_count = byte_count / prdt_payload;

	/* Setup the command header */

	cmd_list = (struct command_list *) ahci_addr_get(&port->command_list);

	cmd_header = &cmd_list->headers[slot];
	cmd_header->cfl = sizeof(struct reg_h2d) / sizeof(pure64_uint32);
	cmd_header->write = 0;
	cmd_header->prdt_length = prdt_count;

	cmd_table = (struct command_table *) ahci_addr_get(&cmd_header->command_table);

	pure64_memset(cmd_table, 0, sizeof(*cmd_table));

	/* Setup the PRDT entries */

	buf8 = (unsigned char *) buf;
	buf_offset = 0;

	for (i = 0; i < prdt_count; i++) {
		/* Set the data destination address */
		ahci_addr_set(&cmd_table->entries[i].data, &buf8[buf_offset]);
		/* Calculate how much data this PRDT will hold */
		if ((byte_count - buf_offset) < prdt_payload)
			cmd_table->entries[i].byte_count = (byte_count - buf_offset) - 1;
		else
			cmd_table->entries[i].byte_count = prdt_payload - 1;
		/* Calculate the new value of the buffer offset */
		buf_offset += cmd_table->entries[i].byte_count + 1;
		/* Set the interrupt bit */
		cmd_table->entries[i].i = 1;
	}

	/* Setup the command FIS */
	cmd_fis = (struct reg_h2d *) &cmd_table->cfis[0];
	/* Set FIS type to "Register FIS - Host to Device" */
	cmd_fis->fis_type = 0x27;
	cmd_fis->c = 1;
	/* Set the ATA command DMA Read Extended */
	cmd_fis->command = 0x25;
	/* Set the sector position */
	cmd_fis->lba0 = (sector >> 0x00) & 0xff;
	cmd_fis->lba1 = (sector >> 0x08) & 0xff;
	cmd_fis->lba2 = (sector >> 0x10) & 0xff;
	cmd_fis->lba3 = (sector >> 0x18) & 0xff;
	cmd_fis->lba4 = (sector >> 0x20) & 0xff;
	cmd_fis->lba5 = (sector >> 0x28) & 0xff;
	/* Set LBA mode */
	cmd_fis->device = 1 << 0x06;
	cmd_fis->countl = (sector_count >> 0) & 0xff;
	cmd_fis->counth = (sector_count >> 8) & 0xff;

	/* Wait until the port is no longer
	 * busy so that we can issue a command */

	/* This variable is here to detect
	 * an unresponsive port */
	spin = 0;
	while (spin++ < 1000000) {
		if (port->tfd & (ATA_DEV_BUSY & ATA_DEV_DRQ))
			continue;
	}

	/* Port is unresponsive */
	if (spin == 1000000)
		return -1;

	/* Issue the command */
	port->ci = 1 << slot;

	/* Wait for the command
	 * to complete */
	while (1) {

		if ((port->ci & (1 << slot)) == 0)
			break;

		if (port->is & TASK_FILE_ERROR) {
			return -1;
		}
	}

	if (port->is & TASK_FILE_ERROR) {
		return -1;
	}

	return 0;
}

/* * * * * * * * * * *
 * AHCI Base Functions
 * * * * * * * * * * */

pure64_uint32 ahci_base_ports_implemented(const volatile struct ahci_base *base) {
	return base->pi;
}

/* * * * * * * * * * * * *
 * AHCI Visitor Functions
 * * * * * * * * * * * * */

static int find_ahci(void *data, pure64_uint8 bus, pure64_uint8 slot) {

	int ret;
	unsigned int i;
	struct ahci_visitor *visitor;
	struct ahci_base *base;
	struct ahci_port *port;

	visitor = (struct ahci_visitor *) data;

	if ((pci_read_class(bus, slot) != PCI_CLASS_STORAGE)
	 && (pci_read_subclass(bus, slot) != PCI_SUBCLASS_SATA)) {
		/* not a ahci controller */
		return 0;
	}

	/* found an ahci controller */

	/* get base address of memory */

	base = (struct ahci_base *) ((pure64_uint64) pci_read_bar5(bus, slot));

	/* notify the visitor of the base */

	if (visitor->visit_base != NULL) {
		ret = visitor->visit_base(visitor->data, base);
		if (ret != 0)
			return ret;
	}

	/* iterate through the ports */

	for (i = 0; i < 32; i++) {

		/* check if the port is implemented */
		if (!(base->pi & (1 << i)))
			continue;

		/* 0x100 is the offset from the base to the first port */
		/* 0x80 is the size of a port */
		port = (struct ahci_port *)(((pure64_uint8 *) base) + 0x100 + (i * 0x80));

		/* notify visitor of port */

		if (visitor->visit_port != NULL) {
			ret = visitor->visit_port(visitor->data, port);
			if (ret != 0)
				return ret;
		}
	}

	return 0;
}

int ahci_visit(struct ahci_visitor *visitor) {

	return pci_visit(find_ahci, visitor);
}

static int stream_read(void *stream_ptr, void *buf, pure64_uint64 size) {

	int err;
	pure64_uint64 sector;
	pure64_uint64 sector_count;
	pure64_uint64 byte;
	struct ahci_stream *stream;

	stream = (struct ahci_stream *) stream_ptr;

	/* Get the sector index */
	sector = stream->position / 512;

	/* Get the byte index within the sector */
	byte = stream->position % 512;

	/* Get the number of sectors to read */
	sector_count = (size + 511) / 512;

	/* Check if the read operation
	 * can be done from the cache */
	if ((sector >= stream->buf_sector)
	 && ((sector + sector_count) <= (stream->buf_sector + stream->buf_sector_count))) {

		/* Calculate the cache position */
		pure64_uint64 cache_pos = (sector - stream->buf_sector) * 512;
		cache_pos += byte;

		/* Copy over the cache data. */
		pure64_memcpy(buf, &stream->buf[cache_pos], size);

		/* Update the stream position. */
		stream->position += size;

		return 0;
	}

	/* If the read operation starts
	 * at the beginning of the sector
	 * and reads less than a sector,
	 * then it should be cached. */
	if (size <= AHCI_CACHE_MAX) {

		unsigned char *tmp = pure64_realloc(stream->buf, sector_count * 512);
		if (tmp == NULL)
			return PURE64_ENOMEM;

		/* Read the data from the port */
		err = ahci_port_read(stream->port, sector, sector_count, stream->buf);
		if (err != 0)
			return PURE64_EIO;

		/* Set the cache parameters */
		stream->buf_sector = sector;
		stream->buf_sector_count = sector_count;

		/* Copy the data to caller buffer */
		pure64_memcpy(buf, &stream->buf[byte], size);

		/* Update the position */
		stream->position += size;

		/* Done */
		return 0;
	} else {
		/* TODO : partitial reads must first
		 * be supported by the stream structure. */
		return PURE64_EIO;
	}

	return 0;
}

static int stream_get_pos(void *stream_ptr, pure64_uint64 *pos) {

	struct ahci_stream *ahci_stream;

	ahci_stream = (struct ahci_stream *) stream_ptr;

	*pos = ahci_stream->position;

	return 0;
}

static int stream_set_pos(void *stream_ptr, pure64_uint64 pos) {

	struct ahci_stream *ahci_stream;

	ahci_stream = (struct ahci_stream *) stream_ptr;

	ahci_stream->position = pos;

	return 0;
}

void ahci_stream_init(struct ahci_stream *stream, volatile struct ahci_port *port) {
	pure64_stream_init(&stream->base);
	stream->base.data = stream;
	stream->base.read = stream_read;
	stream->base.get_pos = stream_get_pos;
	stream->base.set_pos = stream_set_pos;
	stream->port = port;
	stream->position = 0;
	stream->buf = NULL;
	stream->buf_sector = 0;
	stream->buf_sector_count = 0;
}
