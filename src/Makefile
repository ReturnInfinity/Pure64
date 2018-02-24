pure64_deps += init/smp_ap.asm
pure64_deps += init/acpi.asm
pure64_deps += init/cpu.asm
pure64_deps += init/pic.asm
pure64_deps += init/smp.asm
pure64_deps += interrupt.asm
pure64_deps += sysvar.asm

.PHONY: all
all: pure64.sys

pure64.sys: pure64.asm $(pure64_deps)
	@echo "NASM $@"
	nasm $< -o $@

.PHONY: clean
clean:
	$(RM) pure64.sys

$(V).SILENT:
