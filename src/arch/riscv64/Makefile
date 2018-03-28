TOP ?= $(PWD)/../../../

OUTDIR ?= .

VPATH += $(OUTDIR)

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

ifeq (${BOARD},sifive_u500)
CFLAGS += -DPURE64_BOARD_SIFIVE_U500
LDFLAGS += -T sifive_u500.ld
QEMU_MACHINE = sifive_u500
else
CFLAGS += -DPURE64_BOARD_QEMU_VIRT
LDFLAGS += -T qemu_virt.ld
QEMU_MACHINE = virt
endif

.PHONY: all
all: $(OUTDIR)/pure64.sys

$(OUTDIR)/pure64.sys: pure64
	@echo "OBJCOPY $@"
	$(OBJCOPY) -O binary $< $@

$(OUTDIR)/pure64: pure64.o
	@echo "LD      $@"
	$(LD) $(LDFLAGS) $^ -o $@

$(OUTDIR)/pure64.o: pure64.S
	@echo "AS      $@"
	$(AS) $(ASFLAGS) -c $< -o $@

$(OUTDIR)/pure64.img: pure64.sys
	dd if=/dev/zero of=$@ bs=1024 count=1024
	dd if=$< of=$@ conv=notrunc

.PHONY: clean
clean:
	$(RM) $(OUTDIR)/pure64.sys
	$(RM) $(OUTDIR)/pure64
	$(RM) $(OUTDIR)/pure64.o

.PHONY: test
test: pure64.img
	qemu-system-riscv64 -machine $(QEMU_MACHINE) -m 64 -drive file=pure64.img,format=raw -serial stdio

.PHONY: test_debug
test_debug: pure64.img
	qemu-system-riscv64 -machine $(QEMU_MACHINE) -m 64 -drive file=pure64.img,format=raw -serial stdio -s -S

$(V).SILENT:
