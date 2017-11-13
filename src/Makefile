bootsectors += bootsectors/bmfs_mbr.sys
bootsectors += bootsectors/pxestart.sys
bootsectors += bootsectors/multiboot.sys

install_files += $(PREFIX)/system/pure64.sys
install_files += $(PREFIX)/system/bootsectors/bmfs_mbr.sys
install_files += $(PREFIX)/system/bootsectors/pxestart.sys
install_files += $(PREFIX)/system/bootsectors/multiboot.sys

.PHONY: all
all: pure64.sys $(bootsectors)

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

.PHONY: test
test:

.PHONY: install
install: $(install_files)

$(PREFIX)/system/%.sys: %.sys
	cp --update $< $@

$(V).SILENT:
