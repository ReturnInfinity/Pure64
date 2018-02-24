top ?= ../..

VPATH += $(top)/src/lib
VPATH += $(top)/include/pure64

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(top)/include
CFLAGS += -fno-stack-protector
CFLAGS += -fomit-frame-pointer
CFLAGS += -mno-red-zone
CFLAGS += -Os
CFLAGS += -g

LDFLAGS += -T stage-three.ld
LDFLAGS += -L $(top)/src/lib

LDLIBS += -lpure64

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
	objcopy -O binary $< $@

stage-three: $(stage_three_files) stage-three.ld -lpure64
	ld $(stage_three_files) $(LDFLAGS) $(LDLIBS) -o $@

_start.o: _start.c ahci.h debug.h

ahci.o: ahci.c ahci.h pci.h

debug.o: debug.c debug.h

e820.o: e820.c e820.h

hooks.o: hooks.c hooks.h map.h memory.h

map.o: map.c map.h

pci.o: pci.c pci.h

%.o: %.c
	@echo "CC $@"
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
