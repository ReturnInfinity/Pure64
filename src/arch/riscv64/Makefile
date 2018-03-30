TOP ?= ../../../

CROSS_COMPILE=riscv64-none-elf-
AS=$(CROSS_COMPILE)as
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

ASFLAGS += -g

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=c11
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
	@echo "OBJCOPY $@"
	$(OBJCOPY) -O binary $< $@

pure64: pure64.o
	@echo "LD      $@"
	$(LD) $(LDFLAGS) $^ -o $@

pure64.o: pure64.S
	@echo "AS      $@"
	$(AS) $(ASFLAGS) -c $< -o $@

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
