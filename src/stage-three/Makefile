TOP ?= ../..

include $(TOP)/make/patterns.mk
include $(TOP)/make/config.mk

VPATH += $(TOP)/include/pure64
VPATH += $(TOP)/src/lib

CFLAGS += -Os -g
CFLAGS += -ffreestanding

ifdef ARCH_x86_64
CFLAGS += -DPURE64_ARCH_x86_64
endif

ifdef ARCH_riscv64
CFLAGS += -DPURE64_ARCH_riscv64
endif

LDFLAGS += -T loader-$(ARCH).ld
LDFLAGS += -L $(TOP)/src/lib
LDFLAGS += -nostdlib

LDLIBS += -lpure64
LDLIBS += -lgcc

stage_three_files += _start.o
stage_three_files += debug.o
stage_three_files += hooks.o
stage_three_files += map.o

ifdef ARCH_x86_64
stage_three_files += ahci.o
stage_three_files += e820.o
stage_three_files += pci.o
endif

ifdef ARCH_riscv64
stage_three_files += uart.o
stage_three_files += e820-riscv64.o
stage_three_files += ahci-riscv64.o
endif

install_dir = $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)

.PHONY: all
all: stage-three.sys

stage-three.sys: stage-three

stage-three: $(stage_three_files) -lpure64 loader-$(ARCH).ld
	@echo "LD      $@"
	$(LD) $(LDFLAGS) $(stage_three_files) -o $@ $(LDLIBS)

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
	$(RM) $(stage_three_files)
	$(RM) stage-three
	$(RM) stage-three.sys

.PHONY: test
test:

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp stage-three.sys $(install_dir)

$(V).SILENT:
