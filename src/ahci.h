/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_AHCI_H
#define PURE64_AHCI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ahci_port {
	/* command list base address, 1K-byte aligned */
	void *cmd_base;
	/* base address of frame information structure */
	void *fis_base;
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

int ahci_port_is_sata_drive(const struct ahci_port *port);

int ahci_port_read(struct ahci_port *port,
                   uint64_t sector_index,
                   uint64_t sector_count,
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

struct ahci_visitor {
	void *data;
	int (*visit_base)(void *data, struct ahci_base *base);
	int (*visit_port)(void *data, struct ahci_port *port);
};

int ahci_visit(struct ahci_visitor *visitor);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_AHCI_H */
