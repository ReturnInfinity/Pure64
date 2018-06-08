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

pure64-$(PURE64_VERSION).zip: all
	$(MAKE) install PREFIX=$(CURDIR)/pure64-$(PURE64_VERSION)
	zip -9 -y -r -q pure64-$(PURE64_VERSION).zip pure64-$(PURE64_VERSION)

pure64-$(PURE64_VERSION).tar.gz:
	$(MAKE) install PREFIX=$(CURDIR)/pure64-$(PURE64_VERSION)
	tar --create --file pure64-$(PURE64_VERSION).tar pure64-$(PURE64_VERSION)
	gzip pure64-$(PURE64_VERSION).tar

$(V).SILENT:
