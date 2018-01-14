top ?= ../..

VPATH += $(top)/include/pure64

CFLAGS += -I $(top)/include/pure64

.PHONY: all
all: libpure64.a

libpure64.a: dir.o file.o misc.o path.o
	$(AR) $(ARFLAGS) $@ $^

dir.o: dir.c dir.h file.h misc.h path.h

file.o: file.c file.h misc.h

misc.o: misc.c misc.h

path.o: path.c path.h

.PHONY: clean
clean:
	$(RM) dir.o file.o misc.o path.o libpure64.a

.PHONY: test
test:

.PHONY: install
install:
