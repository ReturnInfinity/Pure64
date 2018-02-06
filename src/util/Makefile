top ?= ../..

DESTDIR ?=

PREFIX ?= /usr/local

VPATH += $(top)/src/lib

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(top)/include
CFLAGS += -g

targets += rc
targets += pure64

.PHONY: all
all: $(targets)

rc: rc.o

rc.o: rc.c

pure64: pure64.o pure64-data.o mbr-data.o -lpure64

pure64.o: pure64.c pure64-data.h mbr-data.h

mbr-data.o: mbr-data.c

mbr-data.c mbr-data.h: $(top)/src/bootsectors/bmfs_mbr.sys rc
	./rc --input $< --source mbr-data.c --header mbr-data.h --name mbr_data

pure64-data.o: pure64-data.c

pure64-data.c pure64-data.h: $(top)/src/pure64.sys rc
	./rc --input $< --source pure64-data.c --header pure64-data.h --name pure64_data

.PHONY: clean
clean:
	$(RM) mbr-data.h mbr-data.c mbr-data.o
	$(RM) pure64-data.h pure64-data.c pure64-data.o
	$(RM) pure64.o pure64
	$(RM) rc.o rc

.PHONY: test
test:

install: pure64 $(DESTDIR)$(PREFIX)/bin
	cp pure64 $(DESTDIR)$(PREFIX)/bin/pure64

$(DESTDIR)$(PREFIX)/bin:
	mkdir -p $@

$(V).SILENT:
