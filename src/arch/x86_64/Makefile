pure64_deps += init/smp_ap.asm
pure64_deps += init/acpi.asm
pure64_deps += init/cpu.asm
pure64_deps += init/pic.asm
pure64_deps += init/smp.asm
pure64_deps += interrupt.asm
pure64_deps += sysvar.asm

pure64_install_dir ?= $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)

.PHONY: all
all: pure64.sys

pure64.sys: pure64.asm $(pure64_deps)

%.sys: %.asm
	@echo "NASM    $@"

.PHONY: clean
clean:
	$(RM) pure64.sys

.PHONY: test
test: all

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp pure64.sys $(install_dir)

$(V).SILENT:
