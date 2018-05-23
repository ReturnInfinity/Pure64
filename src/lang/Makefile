TOP ?= ../..

include $(TOP)/make/patterns.mk
include $(TOP)/make/config.mk

VPATH += $(TOP)/include/pure64
VPATH += $(TOP)/include/pure64/lang

objfiles += config.o
objfiles += parser.o
objfiles += scanner.o
objfiles += token.o
objfiles += var.o

targets += libpure64-lang.a

install_dir ?= $(DESTDIR)$(PREFIX)/lib

.PHONY: all
all: $(targets)

libpure64-lang.a: $(objfiles)

config.o: config.c config.h parser.h var.h core/error.h core/types.h

parser.o: parser.c parser.h var.h core/error.h core/types.h

scanner.o: scanner.c scanner.h token.h core/error.h core/types.h

token.o: token.c token.h core/types.h core/error.h

var.o: var.c var.h scanner.h token.h syntax-error.h core/error.h core/types.h

.PHONY: clean
clean:
	$(RM) $(objfiles)
	$(RM) $(targets)

.PHONY: test
test:

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp libpure64-lang.a $(install_dir)

$(V).SILENT:
