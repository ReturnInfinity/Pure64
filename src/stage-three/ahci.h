/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_AHCI_H
#define PURE64_AHCI_H

#include <pure64/stream.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** An AHCI address field.
 * It's a 64-bit value made
 * up of two separate 32-bit
 * integers.
 * */

struct ahci_addr {
	/** The lower 32-bits of the address. */
	uint32_t lower;
	/** The upper 32-bits of the address. */
	uint32_t upper;
};

/** Get the address of an AHCI address field.
 * @param addr An AHCI address structure.
 * @param ptr The pointer to set the address
 * structure value to.
 * */

void ahci_addr_set(volatile struct ahci_addr *addr, void *ptr);

/** Set the address of an AHCI address field.
 * @param addr An AHCI address structure.
 * @returns The pointer constructed from the
 * address field.
 * */

void *ahci_addr_get(const volatile struct ahci_addr *addr);

/** An AHCI port. This structure represents
 * a storage medium in the AHCI specification.
 * It's also called an HBA port.
 * */

struct ahci_port {
	/** The command list base address */
	struct ahci_addr command_list;
	/** The frame information structure
	 * base address */
	struct ahci_addr fis;
	/* interrupt status */
	uint32_t is;
	/* interrupt enable */
	uint32_t ie;
	/* command and status */
	uint32_t cmd;
	/* Reserved */
	uint32_t reserved;
	/* task file data */
	uint32_t tfd;
	/* signature */
	uint32_t sig;
	/* SATA status (SCR0:SStatus) */
	uint32_t ssts;
	/* SATA control (SCR2:SControl) */
	uint32_t sctl;
	/* SATA error (SCR1:SError) */
	uint32_t serr;
	/* SATA active (SCR3:SActive) */
	uint32_t sact;
	/* command issue */
	uint32_t ci;
	/* SATA notification (SCR4:SNotification) */
	uint32_t sntf;
	/* FIS-based switch control */
	uint32_t fbs;
	/* reserved */
	uint32_t reserved1[11];
	/* vendor specific */
	uint32_t vendor[4];
};

int ahci_port_is_sata_drive(const volatile struct ahci_port *port);

int ahci_port_read(volatile struct ahci_port *port,
                   uint64_t sector_index,
                   uint32_t sector_count,
                   void *buffer);

struct ahci_base {
	/* host capability */
	uint32_t cap;
	/* global host control */
	uint32_t ghc;
	/* interrupt status */
	uint32_t is;
	/* port implemented */
	uint32_t pi;
	/* version */
	uint32_t vs;
	/* command completion coalescing control */
	uint32_t ccc_ctl;	// 0x14, Command completion coalescing control
	/* command completion coalescing ports */
	uint32_t ccc_pts;	// 0x18, Command completion coalescing ports
	/* enclosure management location */
	uint32_t em_loc;
	/* enclosure management control */
	uint32_t em_ctl;
	/* host capabilities extended */
	uint32_t cap2;
	/* bios/os control and status */
	uint32_t bohc;
	uint8_t  reserved[0xA0-0x2C];
	uint8_t  vendor_specific[0x100-0xA0];
	struct ahci_port ports[];
};

uint32_t ahci_base_ports_implemented(const volatile struct ahci_base *base);

struct ahci_visitor {
	void *data;
	int (*visit_base)(void *data, volatile struct ahci_base *base);
	int (*visit_port)(void *data, volatile struct ahci_port *port);
};

int ahci_visit(struct ahci_visitor *visitor);

/** A stream wrapper structure so that
 * an AHCI port can be read as a stream.
 * */

struct ahci_stream {
	/** The stream base structure. */
	struct pure64_stream base;
	/** The port associated with
	 * the stream. */
	volatile struct ahci_port *port;
	/** The position of the stream
	 * within the AHCI disk, in bytes. */
	uint64_t position;
	/** A buffer to cache read operations. */
	uint8_t *buf;
	/** The sector of the last cached
	 * read operation. */
	uint64_t buf_sector;
	/** The number of sectors that were
	 * read in the last cached read operation. */
	uint64_t buf_sector_count;
};

/** Initializes an AHCI port stream.
 * @param stream The AHCI stream to initialize
 * @param port The port to associate with the stream.
 * */

void ahci_stream_init(struct ahci_stream *stream, volatile struct ahci_port *port);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_AHCI_H */
