targets += mbr.sys
targets += multiboot.sys
targets += multiboot2.sys
targets += pxestart.sys

.PHONY: all
all: $(targets)

%.sys: %.asm
	@echo "NASM    $@"
	nasm $< -o $@

.PHONY: clean
clean:
	$(RM) $(targets)

.PHONY: test
test:

.PHONY: install
install: $(targets)

$(V).SILENT:
