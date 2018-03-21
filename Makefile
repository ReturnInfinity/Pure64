VERSION ?= 0.9.0

export CROSS_COMPILE ?= x86_64-none-elf-

export HOST_CROSS_COMPILE ?=

.PHONY: all clean install
all clean install:
	$(MAKE) -C include/pure64 $@
	$(MAKE) -C src $@
	$(MAKE) -C src/bootsectors $@
	$(MAKE) -C src/targetlib $@
	$(MAKE) -C src/hostlib $@
	$(MAKE) -C src/stage-three $@
	$(MAKE) -C src/util $@

pure64-$(VERSION).tar.gz: pure64-$(VERSION)
	tar -pcvzf $@ $<

pure64-$(VERSION):
	$(MAKE) install DESTDIR=$(PWD)/$@ PREFIX=/

.PHONY: test
test: test1

.PHONY: test1
test1: testing/test1.img
	./test.sh $<

.PHONY: test2
test2: testing/test2.img
	./test.sh $<

testing/test1.img: testing/test1-config.txt testing/kernel all
	./src/util/pure64 --disk $@ --config $< init

testing/test2.img: testing/test2-config.txt testing/kernel all
	./src/util/pure64 --disk $@ --config $< init
	./src/util/pure64 --disk $@ --config $< cp testing/kernel /boot/kernel

testing/kernel.sys: testing/kernel
	objcopy -O binary $< $@

testing/kernel: testing/kernel.o
	ld $< -o $@

testing/kernel.o: testing/kernel.asm
	nasm $< -f elf64 -o $@

$(V).SILENT:
