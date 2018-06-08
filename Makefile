TOP ?= $(CURDIR)

include $(TOP)/version.mk
include $(TOP)/make/config.mk
include $(TOP)/make/patterns.mk

.PHONY: all clean test install
all clean test install:
	$(MAKE) -C src/arch/$(ARCH) $@
	$(MAKE) -C src/core $@
	$(MAKE) -C src/lang $@
	$(MAKE) -C src/fs-loader $@
	$(MAKE) -C src/util $@

pure64-$(PURE64_VERSION).zip:
	$(MAKE) all-pure64 CROSS_COMPILE=x86_64-w64-mingw32- EXE=.exe
	$(MAKE) install-pure64 CROSS_COMPILE=x86_64-w64-mingw32- EXE=.exe PREFIX=$(CURDIR)/pure64-$(PURE64_VERSION)
	$(MAKE) clean-pure64 EXE=.exe
	$(MAKE) all-pure64-x86_64
	$(MAKE) install-pure64-x86_64 PREFIX=$(CURDIR)/pure64-$(PURE64_VERSION)
	$(MAKE) clean-pure64-x86_64
	zip -9 -y -r -q pure64-$(PURE64_VERSION).zip pure64-$(PURE64_VERSION)
	$(MAKE) clean-pure64

pure64-$(PURE64_VERSION).tar.gz:
	$(MAKE) all-pure64
	$(MAKE) install-pure64 PREFIX=$(CURDIR)/pure64-$(PURE64_VERSION)
	$(MAKE) clean-pure64
	$(MAKE) all-pure64-x86_64
	$(MAKE) install-pure64-x86_64 PREFIX=$(CURDIR)/pure64-$(PURE64_VERSION)
	$(MAKE) clean-pure64-x86_64
	tar --create --file pure64-$(PURE64_VERSION).tar pure64-$(PURE64_VERSION)
	gzip pure64-$(PURE64_VERSION).tar
	$(MAKE) clean-pure64

$(V).SILENT:
