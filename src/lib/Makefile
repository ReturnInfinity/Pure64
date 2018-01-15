top ?= ../..

DESTDIR ?=

PREFIX ?= /usr/local

VPATH += $(top)/include/pure64

CFLAGS += -I $(top)/include
CFLAGS += -fno-stack-protector
CFLAGS += -fomit-frame-pointer
CFLAGS += -g

libfiles += dir.o
libfiles += file.o
libfiles += fs.o
libfiles += misc.o
libfiles += path.o
libfiles += stream.o

targets += libpure64.a

.PHONY: all
all: $(targets)

libpure64.a: $(libfiles)
	$(AR) $(ARFLAGS) $@ $^

dir.o: dir.c dir.h file.h misc.h path.h

file.o: file.c file.h misc.h

fs.o: fs.c fs.h file.h dir.h path.h misc.h

misc.o: misc.c misc.h

path.o: path.c path.h

stream.o: stream.c stream.h

.PHONY: clean
clean:
	$(RM) $(libfiles)
	$(RM) libpure64.a

.PHONY: test
test:

.PHONY: install
install: $(DESTDIR)$(PREFIX)/lib/libpure64.a

$(DESTDIR)$(PREFIX)/lib:
	mkdir $@

$(DESTDIR)$(PREFIX)/lib/libpure64.a: $(DESTDIR)$(PREFIX)/lib libpure64.a
	cp $< $@

$(V).SILENT:
