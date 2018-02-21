/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include "pci.h"

#include <stdint.h>

static void out32(uint16_t port, uint32_t value) {
	asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t in32(uint16_t port) {
	uint32_t value;
	asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

uint32_t pci_read_bar5(uint8_t bus,
                       uint8_t slot) {
	return pci_read(bus, slot, 0, 0x24);
}

uint8_t pci_read_class(uint8_t bus,
                       uint8_t slot) {
	return (pci_read(bus, slot, 0, 8) & 0xff000000) >> 24;
}

uint8_t pci_read_interface(uint8_t bus,
                           uint8_t slot) {
	return (pci_read(bus, slot, 0, 8) & 0xff00) >> 8;
}

uint8_t pci_read_subclass(uint8_t bus,
                          uint8_t slot) {
	return (pci_read(bus, slot, 0, 8) & 0xff0000) >> 16;
}

uint16_t pci_read_vendor(uint8_t bus,
                         uint8_t slot) {
	return pci_read(bus, slot, 0, 0) & 0xffff;
}

uint32_t pci_read(uint8_t bus,
                  uint8_t slot,
                  uint8_t func,
                  uint8_t reg) {

	uint32_t address;

	address = 0;
	address |= ((uint32_t) bus) << 16;
	address |= ((uint32_t) slot) << 11;
	address |= ((uint32_t) func) << 8;
	address |= ((uint32_t) reg) & 0xfc;
	address |= 0x80000000;
 
	out32(PCI_CONFIG_ADDRESS, address);

	return in32(PCI_CONFIG_DATA);
}

int pci_visit(int (*func)(void *, uint8_t, uint8_t), void *data) {

	int ret;
	uint8_t bus;
	uint8_t slot;
	uint16_t vendor;

	if (!func)
		return 0;

	/* use brute force method to scan for
	 * pci devices */

	for (bus = 0; bus < 255; bus++) {
		for (slot = 0; slot < 32; slot++) {
			vendor = pci_read_vendor(bus, slot);
			if (vendor == 0xffff)
				break;

			ret = func(data, bus, slot);
			if (ret != 0)
				return ret;
		}
	}

	return 0;
}
