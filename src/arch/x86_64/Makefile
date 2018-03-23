pure64_deps += init/smp_ap.asm
pure64_deps += init/acpi.asm
pure64_deps += init/cpu.asm
pure64_deps += init/pic.asm
pure64_deps += init/smp.asm
pure64_deps += interrupt.asm
pure64_deps += sysvar.asm

bootsectors += bootsectors/mbr.sys
bootsectors += bootsectors/multiboot.sys
bootsectors += bootsectors/multiboot2.sys
bootsectors += bootsectors/pxestart.sys

.PHONY: all
all: pure64.sys $(bootsectors)

pure64.sys: pure64.asm $(pure64_deps)

%.sys: %.asm
	@echo "NASM    $@"
	nasm $< -o $@

.PHONY: clean
clean:
	$(RM) pure64.sys
	$(RM) $(bootsectors)

.PHONY: test
test: all

.PHONY: install
install: all

$(V).SILENT:
