.PHONY: all
all: all-pure64

.PHONY: clean
clean: clean-pure64

.PHONY: test
test: test-pure64

.PHONY: all-pure64
all-pure64:
	$(MAKE) -C src/lib all
	$(MAKE) -C src/util all

.PHONY: clean-pure64
clean-pure64:
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/util clean

.PHONY: test-pure64
test-pure64: all-pure64
	$(MAKE) -C src/util test

.PHONY: install-pure64
install-pure64: all-pure64
	$(MAKE) -C src/lib install
	$(MAKE) -C src/util install

.PHONY: all-pure64-x86_64
all-pure64-x86_64:
	$(MAKE) -C src/arch/x86_64/bootsectors all
	$(MAKE) -C src/arch/x86_64 all
	$(MAKE) -C src/lib all
	$(MAKE) -C src/stage-three all

.PHONY: clean-pure64-x86_64
clean-pure64-x86_64:
	$(MAKE) -C src/arch/x86_64/bootsectors clean
	$(MAKE) -C src/arch/x86_64 clean
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/stage-three clean

.PHONY: install-pure64-x86_64
install-pure64-x86_64:
	$(MAKE) -C src/arch/x86_64/bootsectors install
	$(MAKE) -C src/arch/x86_64 install
	$(MAKE) -C src/lib install
	$(MAKE) -C src/stage-three install

.PHONY: all-pure64-riscv64
all-pure64-riscv64:
	$(MAKE) -C src/arch/riscv64 all
	$(MAKE) -C src/lib all
	$(MAKE) -C src/stage-three all

.PHONY: clean-pure64-riscv64
clean-pure64-riscv64:
	$(MAKE) -C src/arch/riscv64 clean
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/stage-three clean

.PHONY: install-pure64-riscv64
install-pure64-riscv64:
	$(MAKE) -C src/arch/riscv64 install
	$(MAKE) -C src/lib install
	$(MAKE) -C src/stage-three install

$(V).SILENT:
