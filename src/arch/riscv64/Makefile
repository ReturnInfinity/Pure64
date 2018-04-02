TOP ?= ../../../

include $(TOP)/make/config.mk
include $(TOP)/make/patterns.mk

ASFLAGS += -g

CFLAGS += -g
CFLAGS += -ffreestanding
CFLAGS += -I $(TOP)/include

ifeq ($(BOARD),sifive_u500)
CFLAGS += -DPURE64_BOARD_SIFIVE_U500
LDFLAGS += -T sifive_u500.ld
QEMU_MACHINE = sifive_u500
else
CFLAGS += -DPURE64_BOARD_QEMU_VIRT
LDFLAGS += -T qemu_virt.ld
QEMU_MACHINE = virt
endif

.PHONY: all
all: pure64.sys

pure64.sys: pure64

pure64: pure64.o

pure64.o: pure64.S

.PHONY: clean
clean:
	$(RM) pure64.sys
	$(RM) pure64
	$(RM) pure64.o

.PHONY: test
test:

.PHONY: install
install: pure64.sys
	mkdir -p $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)
	cp pure64.sys $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)/pure64.sys

$(V).SILENT:
