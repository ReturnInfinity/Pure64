VERSION ?= 0.9.0

.PHONY: all clean install
all clean install:
	$(MAKE) -C src $@
	$(MAKE) -C src/bootsectors $@
	$(MAKE) -C src/lib $@
	$(MAKE) -C src/util $@
	$(MAKE) -C include/pure64 $@

pure64-$(VERSION).tar.gz: pure64-$(VERSION)
	tar -pcvzf $@ $<

pure64-$(VERSION):
	$(MAKE) install DESTDIR=$(PWD)/$@ PREFIX=/

.PHONY: test
test: pure64.img
	./test.sh

pure64.img: all
	./src/util/pure64 mkfs

$(V).SILENT:
