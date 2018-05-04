TOP ?= $(CURDIR)

include $(TOP)/version.mk
include $(TOP)/make/config.mk
include $(TOP)/make/patterns.mk

-include config.mk

default_target ?= pure64

.PHONY: all
all: all-$(default_target)

.PHONY: clean
clean: clean-$(default_target)

.PHONY: test
test: test-$(default_target)

.PHONY: install
install: install-$(default_target)

.PHONY: all-pure64
all-pure64:
	$(MAKE) -C src/core all
	$(MAKE) -C src/lang all
	$(MAKE) -C src/util all

.PHONY: clean-pure64
clean-pure64:
	$(MAKE) -C src/core clean
	$(MAKE) -C src/lang clean
	$(MAKE) -C src/util clean

.PHONY: test-pure64
test-pure64: all-pure64
	$(MAKE) -C src/util test

.PHONY: install-pure64
install-pure64: all-pure64
	$(MAKE) -C src/core install
	$(MAKE) -C src/lang install
	$(MAKE) -C src/util install

.PHONY: all-pure64-x86_64
all-pure64-x86_64:
	$(MAKE) -C src/arch/x86_64/bootsectors all
	$(MAKE) -C src/arch/x86_64 all
	$(MAKE) -C src/core all CROSS_COMPILE=x86_64-none-elf- ARCH=x86_64
	$(MAKE) -C src/fs-loader all CROSS_COMPILE=x86_64-none-elf- ARCH=x86_64

.PHONY: clean-pure64-x86_64
clean-pure64-x86_64:
	$(MAKE) -C src/arch/x86_64/bootsectors clean
	$(MAKE) -C src/arch/x86_64 clean
	$(MAKE) -C src/core clean ARCH=x86_64
	$(MAKE) -C src/fs-loader clean ARCH=x86_64

.PHONY: install-pure64-x86_64
install-pure64-x86_64:
	$(MAKE) -C src/arch/x86_64/bootsectors install
	$(MAKE) -C src/arch/x86_64 install
	$(MAKE) -C src/core install CROSS_COMPILE=x86_64-none-elf- ARCH=x86_64
	$(MAKE) -C src/fs-loader install CROSS_COMPILE=x86_64-none-elf- ARCH=x86_64

.PHONY: all-pure64-riscv64
all-pure64-riscv64:
	$(MAKE) -C src/arch/riscv64 all
	$(MAKE) -C src/core all CROSS_COMPILE=riscv64-none-elf- ARCH=riscv64
	$(MAKE) -C src/fs-loader all CROSS_COMPILE=riscv64-none-elf- ARCH=riscv64

.PHONY: clean-pure64-riscv64
clean-pure64-riscv64:
	$(MAKE) -C src/arch/riscv64 clean
	$(MAKE) -C src/core clean ARCH=riscv64
	$(MAKE) -C src/fs-loader clean ARCH=riscv64

.PHONY: install-pure64-riscv64
install-pure64-riscv64:
	$(MAKE) -C src/arch/riscv64 install
	$(MAKE) -C src/core install CROSS_COMPILE=riscv64-none-elf- ARCH=riscv64
	$(MAKE) -C src/fs-loader install CROSS_COMPILE=riscv64-none-elf- ARCH=riscv64

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
