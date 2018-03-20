VERSION ?= 0.9.0

.PHONY: all clean install
all clean install:
	$(MAKE) -C include/pure64 $@
	$(MAKE) -C src $@
	$(MAKE) -C src/bootsectors $@
	$(MAKE) -C src/lib $@
	$(MAKE) -C src/stage-three $@
	$(MAKE) -C src/util $@

pure64-$(VERSION).tar.gz: pure64-$(VERSION)
	tar -pcvzf $@ $<

pure64-$(VERSION):
	$(MAKE) install DESTDIR=$(PWD)/$@ PREFIX=/

.PHONY: test
test: pure64.img
	./test.sh

pure64.img: all testing/kernel examples/example2-config.txt
	./src/util/pure64 --config examples/example2-config.txt init
	./src/util/pure64 --config examples/example2-config.txt cp testing/kernel /boot/kernel

testing/kernel.sys: testing/kernel
	objcopy -O binary $< $@

testing/kernel: testing/kernel.o
	ld $< -o $@

testing/kernel.o: testing/kernel.asm
	nasm $< -f elf64 -o $@

$(V).SILENT:
