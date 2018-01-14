top ?= ..

VPATH += $(top)/include/pure64
VPATH += lib
VPATH += util

CFLAGS += -I $(top)/include

bootsectors += bootsectors/bmfs_mbr.sys
bootsectors += bootsectors/pxestart.sys
bootsectors += bootsectors/multiboot.sys

libfiles += lib/dir.o
libfiles += lib/file.o
libfiles += lib/misc.o
libfiles += lib/path.o

install_files += $(PREFIX)/system/pure64.sys
install_files += $(PREFIX)/system/bootsectors/bmfs_mbr.sys
install_files += $(PREFIX)/system/bootsectors/pxestart.sys
install_files += $(PREFIX)/system/bootsectors/multiboot.sys

targets += util/pure64
targets += lib/libpure64.a
targets += pure64.sys
targets += $(bootsectors)

.PHONY: all
all: $(targets)

util/pure64: util/pure64.o -lpure64

util/pure64.o: util/pure64.c

lib/libpure64.a: $(libfiles)
	$(AR) $(ARFLAGS) $@ $^

lib/dir.o: lib/dir.c dir.h file.h misc.h path.h

lib/file.o: lib/file.c file.h misc.h

lib/misc.o: lib/misc.c misc.h

lib/path.o: lib/path.c path.h

.PHONY: install
install:
# The dependencies of pure64.sys can
# be (and should be) generated with
# the command: nasm -M pure64.asm 1>pure64.mk
# and used to update this target.
pure64.sys: \
	pure64.asm \
	init/smp_ap.asm \
	init/acpi.asm \
	init/cpu.asm \
	init/pic.asm \
	init/smp.asm \
	interrupt.asm \
	sysvar.asm

%.sys: %.asm
	@echo "NASM $@"
	nasm $< -o $@

.PHONY: clean
clean:
	$(RM) pure64.sys $(bootsectors)
	$(RM) $(libfiles) lib/libpure64.a
	$(RM) util/pure64.o util/pure64

.PHONY: test
test:

.PHONY: install
install: $(install_files)

$(PREFIX)/system/%.sys: %.sys
	cp --update $< $@

$(V).SILENT:
