bootsectors += mbr.sys
bootsectors += multiboot.sys
bootsectors += multiboot2.sys
bootsectors += pxestart.sys

listfiles += mbr.lst
listfiles += multiboot.lst
listfiles += multiboot2.lst
listfiles += pxestart.lst

ARCH ?= x86_64

install_dir ?= $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)/bootsectors

.PHONY: all
all: $(bootsectors)

mbr.sys: mbr.asm
	@echo "NASM    $@"
	nasm $< -o $@ -l mbr.lst

pxestart.sys: pxestart.asm
	@echo "NASM    $@"
	nasm $< -o $@ -l pxestart.lst

multiboot.sys: multiboot.asm
	@echo "NASM    $@"
	nasm $< -o $@ -l multiboot.lst

multiboot2.sys: multiboot2.asm
	@echo "NASM    $@"
	nasm $< -o $@ -l multiboot2.lst

.PHONY: clean
clean:
	$(RM) $(bootsectors)
	$(RM) $(listfiles)

.PHONY: test
test: all

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp $(bootsectors) $(install_dir)
	cp $(listfiles) $(install_dir)

$(V).SILENT:
