TOP ?= ../../..

include $(TOP)/make/config.mk
include $(TOP)/make/patterns.mk

pure64_deps += init/smp_ap.asm
pure64_deps += init/acpi.asm
pure64_deps += init/cpu.asm
pure64_deps += init/pic.asm
pure64_deps += init/smp.asm
pure64_deps += interrupt.asm
pure64_deps += sysvar.asm

bootsectors += bootsectors/mbr.sys
bootsectors += bootsectors/pxestart.sys
bootsectors += bootsectors/multiboot.sys
bootsectors += bootsectors/multiboot2.sys

ARCH ?= x86_64

install_dir ?= $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)

NASM ?= nasm

.PHONY: all
all: pure64.sys $(bootsectors)

pure64.sys: pure64.asm $(pure64_deps)

.PHONY: clean
clean:
	$(RM) pure64.sys $(bootsectors)
	$(RM) pure64.lst bootsectors/*.lst

.PHONY: test
test: all

.PHONY: install
install: all
	mkdir -p $(install_dir)
	mkdir -p $(install_dir)/bootsectors
	cp pure64.sys $(install_dir)
	cp pure64.lst $(install_dir)
	cp -R $(bootsectors) $(install_dir)/bootsectors

$(V).SILENT:
