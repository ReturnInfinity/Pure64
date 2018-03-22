top ?= ../..

VPATH += $(top)/src/targetlib
VPATH += $(top)/include/pure64

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -I $(top)/include
CFLAGS += -Os -g
CFLAGS += -ffreestanding

LDFLAGS += -T stage-three.ld
LDFLAGS += -L $(top)/src/targetlib
LDFLAGS += -nostdlib

LDLIBS += -lpure64
LDLIBS += -lgcc

stage_three_files += _start.o
stage_three_files += ahci.o
stage_three_files += debug.o
stage_three_files += e820.o
stage_three_files += hooks.o
stage_three_files += map.o
stage_three_files += pci.o

.PHONY: all
all: stage-three.sys

stage-three.sys: stage-three
	@echo "OBJCOPY $@"
	$(OBJCOPY) -O binary $< $@

stage-three: $(stage_three_files) stage-three.ld -lpure64
	@echo "LD      $@"
	$(LD) $(stage_three_files) $(LDFLAGS) $(LDLIBS) -o $@

_start.o: _start.c ahci.h debug.h

ahci.o: ahci.c ahci.h pci.h

debug.o: debug.c debug.h

e820.o: e820.c e820.h

hooks.o: hooks.c hooks.h map.h memory.h

map.o: map.c map.h

pci.o: pci.c pci.h

%.o: %.c
	@echo "CC      $@"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(stage_three_files)
	$(RM) stage-three stage-three.sys

.PHONY: test
test:

.PHONY: install
install: stage-three.sys

$(V).SILENT:
