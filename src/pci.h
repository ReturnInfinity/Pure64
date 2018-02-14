/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#ifndef PURE64_PCI_H
#define PURE64_PCI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PCI_CONFIG_DATA
#define PCI_CONFIG_DATA 0xcfc
#endif

#ifndef PCI_CONFIG_ADDRESS
#define PCI_CONFIG_ADDRESS 0xcf8
#endif

#ifndef PCI_CLASS_STORAGE
#define PCI_CLASS_STORAGE 0x01
#endif

#ifndef PCI_SUBCLASS_SATA
#define PCI_SUBCLASS_SATA 0x06
#endif

int pci_visit(int (*callback)(void *, uint8_t, uint8_t), void *data);

uint32_t pci_read(uint8_t bus,
                  uint8_t slot,
                  uint8_t func,
                  uint8_t offset);

uint32_t pci_read_bar5(uint8_t bus,
                       uint8_t slot);

uint8_t pci_read_class(uint8_t bus,
                       uint8_t slot);

uint8_t pci_read_interface(uint8_t bus,
                           uint8_t slot);

uint8_t pci_read_subclass(uint8_t bus,
                          uint8_t slot);

uint16_t pci_read_vendor(uint8_t bus,
                         uint8_t slot);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_PCI_H */
