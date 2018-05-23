TOP ?= ../..

include $(TOP)/make/patterns.mk
include $(TOP)/make/config.mk

VPATH += $(TOP)/src/core
VPATH += $(TOP)/src/lang

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(TOP)/include
CFLAGS += -g

targets += pure64$(EXE)

pure64_files += pure64.o
pure64_files += fstream.o
pure64_files += memory.o
pure64_files += util.o

install_dir ?= $(DESTDIR)$(PREFIX)/bin

.PHONY: all
all: $(targets)

pure64$(EXE): $(pure64_files) -lpure64-core -lpure64-lang

pure64.o: pure64.c fstream.h util.h

fstream.o: fstream.c fstream.h

memory.o: memory.c

util.o: util.c util.h fstream.h

.PHONY: clean
clean:
	$(RM) pure64$(EXE) $(pure64_files)

.PHONY: test
test:

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp pure64$(EXE) $(install_dir)

$(V).SILENT:
