/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "ahci.h"

#include "pci.h"

int ahci_port_is_sata_drive(const struct ahci_port *port) {
	return port->sig == 0x101;
}

static int find_ahci(void *data, uint8_t bus, uint8_t slot) {

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

	base = (struct ahci_base *) ((uint64_t) pci_read_bar5(bus, slot));

	/* notify the visitor of the base */

	if (visitor->visit_base != NULL) {
		ret = visitor->visit_base(visitor->data, base);
		if (ret != 0)
			return ret;
	}

	/* iterate through the ports */

	for (i = 0; i < 32; i++) {

		/* 0x100 is the offset from the base to the first port */
		/* 0x80 is the size of a port */
		port = (struct ahci_port *)(((uint8_t *) base) + 0x100 + (i * 0x80));

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
