TOP ?= ../..
PREFIX ?= /opt/return-infinity
VPATH += $(TOP)/include/pure64

include $(TOP)/make/patterns.mk

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(TOP)/include
CFLAGS += -g

ARFLAGS = rcs

libfiles += dap.o
libfiles += dir.o
libfiles += error.o
libfiles += file.o
libfiles += fs.o
libfiles += gpt.o
libfiles += mbr.o
libfiles += misc.o
libfiles += path.o
libfiles += partition.o
libfiles += stream.o
libfiles += string.o
libfiles += uuid.o

targets += libpure64.a

install_dir ?= $(DESTDIR)$(PREFIX)/lib

.PHONY: all
all: $(targets)

libpure64.a: $(libfiles)

dap.o: dap.c dap.h misc.h stream.h

dir.o: dir.c dir.h file.h misc.h path.h

error.o: error.c error.h

file.o: file.c file.h misc.h

fs.o: fs.c fs.h file.h dir.h path.h misc.h

gpt.o: gpt.c gpt.h string.h stream.h misc.h

mbr.o: mbr.c mbr.h string.h stream.h misc.h dap.h

misc.o: misc.c misc.h

path.o: path.c path.h

partition.o: partition.c partition.h stream.h

stream.o: stream.c stream.h

string.o: string.c string.h

uuid.o: uuid.c uuid.h string.h

.PHONY: clean
clean:
	$(RM) $(libfiles)
	$(RM) $(targets)

.PHONY: test
test:

.PHONY: install
install: all
	mkdir -p $(install_dir)
	cp libpure64.a $(install_dir)

$(V).SILENT:
