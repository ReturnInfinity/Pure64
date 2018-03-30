bootsectors += mbr.sys
bootsectors += multiboot.sys
bootsectors += multiboot2.sys
bootsectors += pxestart.sys

install_dir ?= $(DESTDIR)$(PREFIX)/share/pure64/resources/$(ARCH)/bootsectors

.PHONY: all
all: $(bootsectors)

pure64.sys: pure64.asm $(pure64_deps)

%.sys: %.asm
	@echo "NASM    $@"
	nasm $< -o $@

.PHONY: clean
clean:
	$(RM) $(bootsectors)

.PHONY: test
test: all

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp $(bootsectors) $(install_dir)

$(V).SILENT:
