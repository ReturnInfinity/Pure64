CROSS_COMPILE=riscv64-none-elf-
AS=$(CROSS_COMPILE)as
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld

ASFLAGS += -g

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=c11
CFLAGS += -g
CFLAGS += -ffreestanding

ifeq (${BOARD},sifive_u500)
CFLAGS += -DPURE64_BOARD_SIFIVE_U500
LDFLAGS += -T sifive_u500.ld
QEMU_MACHINE = sifive_u500
else
CFLAGS += -DPURE64_BOARD_QEMU_VIRT
LDFLAGS += -T qemu_virt.ld
QEMU_MACHINE = virt
endif


pure64_files += _start.o
pure64_files += main.o
pure64_files += uart.o

.PHONY: all
all: pure64

pure64: $(pure64_files)
	$(LD) $(LDFLAGS) $^ -o $@

_start.o: _start.S

main.o: main.c uart.h

sifive_u500.o: sifive_u500.c sifive_u500.h

uart.o: uart.c uart.h sifive_u500.h

.PHONY: clean
clean:
	$(RM) pure64
	$(RM) *.o

.PHONY: test
test:
	qemu-system-riscv64 -machine $(QEMU_MACHINE) -m 64 -kernel pure64 -serial stdio

.PHONY: test_debug
test_debug:
	qemu-system-riscv64 -machine $(QEMU_MACHINE) -m 64 -kernel pure64 -serial stdio -s -S
