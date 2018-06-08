TOP ?= ../..

include $(TOP)/make/patterns.mk
include $(TOP)/make/config.mk

VPATH += $(TOP)/include/pure64/core
VPATH += $(TOP)/src/core

CFLAGS += -Os -g
CFLAGS += -ffreestanding

ifdef ARCH_x86_64
CFLAGS += -DPURE64_ARCH_x86_64
endif

ifdef ARCH_riscv64
CFLAGS += -DPURE64_ARCH_riscv64
endif

LDFLAGS += -T loader-$(ARCH).ld
LDFLAGS += -L $(TOP)/src/core
LDFLAGS += -nostdlib

LDLIBS += -lpure64-core
LDLIBS += -lgcc

fs_loader_files += _start.o
fs_loader_files += debug.o
fs_loader_files += hooks.o
fs_loader_files += map.o

ifdef ARCH_x86_64
fs_loader_files += ahci.o
fs_loader_files += e820.o
fs_loader_files += pci.o
endif

ifdef ARCH_riscv64
fs_loader_files += uart.o
fs_loader_files += e820-riscv64.o
fs_loader_files += ahci-riscv64.o
endif

install_dir = $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)

.PHONY: all
all: fs-loader.sys

fs-loader.sys: fs-loader

fs-loader: $(fs_loader_files) -lpure64-core loader-$(ARCH).ld
	@echo "LD      $@"
	$(LD) $(LDFLAGS) $(fs_loader_files) -o $@ $(LDLIBS)

_start.o: _start.c ahci.h debug.h

ahci.o: ahci.c ahci.h pci.h

ahci-riscv64.o: ahci-riscv64.c ahci.h

debug.o: debug.c debug.h

e820.o: e820.c e820.h

e820-riscv64.o: e820-riscv64.c e820.h

hooks.o: hooks.c hooks.h map.h memory.h

map.o: map.c map.h

pci.o: pci.c pci.h

uart.o: uart.c uart.h

.PHONY: clean
clean:
	$(RM) $(fs_loader_files)
	$(RM) fs-loader
	$(RM) fs-loader.sys

.PHONY: test
test:

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp fs-loader.sys $(install_dir)

$(V).SILENT:
