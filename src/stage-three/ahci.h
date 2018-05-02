/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_AHCI_H
#define PURE64_AHCI_H

#include <pure64/core/stream.h>
#include <pure64/core/types.h>

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
	pure64_uint32 lower;
	/** The upper 32-bits of the address. */
	pure64_uint32 upper;
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
	pure64_uint32 is;
	/* interrupt enable */
	pure64_uint32 ie;
	/* command and status */
	pure64_uint32 cmd;
	/* Reserved */
	pure64_uint32 reserved;
	/* task file data */
	pure64_uint32 tfd;
	/* signature */
	pure64_uint32 sig;
	/* SATA status (SCR0:SStatus) */
	pure64_uint32 ssts;
	/* SATA control (SCR2:SControl) */
	pure64_uint32 sctl;
	/* SATA error (SCR1:SError) */
	pure64_uint32 serr;
	/* SATA active (SCR3:SActive) */
	pure64_uint32 sact;
	/* command issue */
	pure64_uint32 ci;
	/* SATA notification (SCR4:SNotification) */
	pure64_uint32 sntf;
	/* FIS-based switch control */
	pure64_uint32 fbs;
	/* reserved */
	pure64_uint32 reserved1[11];
	/* vendor specific */
	pure64_uint32 vendor[4];
};

int ahci_port_is_sata_drive(const volatile struct ahci_port *port);

int ahci_port_read(volatile struct ahci_port *port,
                   pure64_uint64 sector_index,
                   pure64_uint32 sector_count,
                   void *buffer);

struct ahci_base {
	/* host capability */
	pure64_uint32 cap;
	/* global host control */
	pure64_uint32 ghc;
	/* interrupt status */
	pure64_uint32 is;
	/* port implemented */
	pure64_uint32 pi;
	/* version */
	pure64_uint32 vs;
	/* command completion coalescing control */
	pure64_uint32 ccc_ctl;	// 0x14, Command completion coalescing control
	/* command completion coalescing ports */
	pure64_uint32 ccc_pts;	// 0x18, Command completion coalescing ports
	/* enclosure management location */
	pure64_uint32 em_loc;
	/* enclosure management control */
	pure64_uint32 em_ctl;
	/* host capabilities extended */
	pure64_uint32 cap2;
	/* bios/os control and status */
	pure64_uint32 bohc;
	pure64_uint8  reserved[0xA0-0x2C];
	pure64_uint8  vendor_specific[0x100-0xA0];
	struct ahci_port ports[];
};

pure64_uint32 ahci_base_ports_implemented(const volatile struct ahci_base *base);

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
	pure64_uint64 position;
	/** A buffer to cache read operations. */
	pure64_uint8 *buf;
	/** The sector of the last cached
	 * read operation. */
	pure64_uint64 buf_sector;
	/** The number of sectors that were
	 * read in the last cached read operation. */
	pure64_uint64 buf_sector_count;
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
