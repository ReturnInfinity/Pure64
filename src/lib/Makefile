top ?= ../..

DESTDIR ?=

PREFIX ?= /usr/local

VPATH += $(top)/include/pure64

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(top)/include
CFLAGS += -fno-stack-protector
CFLAGS += -fomit-frame-pointer
CFLAGS += -g

ARFLAGS = rcs

libfiles += dap.o
libfiles += dir.o
libfiles += error.o
libfiles += file.o
libfiles += fs.o
libfiles += mbr.o
libfiles += misc.o
libfiles += path.o
libfiles += stream.o
libfiles += string.o
libfiles += uuid.o

targets += libpure64.a

.PHONY: all
all: $(targets)

libpure64.a: $(libfiles)
	@echo "AR $@"
	$(AR) $(ARFLAGS) $@ $^

dap.o: dap.c dap.h misc.h stream.h

dir.o: dir.c dir.h file.h misc.h path.h

error.o: error.c error.h

file.o: file.c file.h misc.h

fs.o: fs.c fs.h file.h dir.h path.h misc.h

mbr.o: mbr.c mbr.h string.h stream.h misc.h dap.h

misc.o: misc.c misc.h

path.o: path.c path.h

stream.o: stream.c stream.h

string.o: string.c string.h

uuid.o: uuid.c uuid.h string.h

%.o: %.c
	@echo "CC $@"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(libfiles)
	$(RM) libpure64.a

.PHONY: test
test:

.PHONY: install
install: $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/lib/libpure64.a

$(DESTDIR)$(PREFIX)/lib:
	mkdir -p $@

$(DESTDIR)$(PREFIX)/lib/libpure64.a: libpure64.a
	cp $< $@

$(V).SILENT:
