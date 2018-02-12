top ?= ..

NASM ?= nasm
NASMFLAGS = -f elf64 -F dwarf

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(top)/include
CFLAGS += -fno-stack-protector
CFLAGS += -fomit-frame-pointer
CFLAGS += -mno-red-zone
CFLAGS += -g

targets += pure64.sys

pure64files += pure64.asm
pure64files += init/smp_ap.asm
pure64files += init/acpi.asm
pure64files += init/cpu.asm
pure64files += init/pic.asm
pure64files += init/smp.asm
pure64files += interrupt.asm
pure64files += sysvar.asm

.PHONY: all
all: $(targets)

pure64: pure64.o debug.o load.o pure64.ld
	ld pure64.o debug.o load.o -T pure64.ld -o $@

# The dependencies of pure64.sys can
# be (and should be) generated with
# the command: nasm -M pure64.asm 1>pure64.mk
# and used to update this target.
pure64.o: $(pure64files)

debug.o: debug.c debug.h

load.o: load.c

pure64.sys: pure64
	objcopy -O binary $< $@

%.o: %.asm
	@echo "NASM $@"
	$(NASM) $(NASMFLAGS) $< -o $@

%.o: %.c
	@echo "CC $@"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) load.o pure64.o
	$(RM) pure64 pure64.sys

.PHONY: test
test:

.PHONY: install
install: $(targets)

$(V).SILENT:
