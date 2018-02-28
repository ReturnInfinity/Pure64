top ?= ../..

DESTDIR ?=

PREFIX ?= /usr/local

VPATH += $(top)/src/lib

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(top)/include
CFLAGS += -g

targets += rc

ifdef WIN32
targets += pure64.exe
else
targets += pure64
endif

.PHONY: all
all: $(targets)

rc: rc.o

rc.o: rc.c

pure64: pure64.o stage-three-data.o pure64-data.o mbr-data.o fstream.o -lpure64

pure64.exe: pure64.o stage-three-data.o pure64-data.o mbr-data.o -lpure64
	x86_64-w64-mingw32-ld $^ -o $@

pure64.o: pure64.c stage-three-data.h pure64-data.h mbr-data.h fstream.h

fstream.o: fstream.c fstream.h

mbr-data.o: mbr-data.c

mbr-data.c mbr-data.h: $(top)/src/bootsectors/mbr.sys rc
	./rc --input $< --source mbr-data.c --header mbr-data.h --name mbr_data

pure64-data.o: pure64-data.c

pure64-data.c pure64-data.h: $(top)/src/pure64.sys rc
	./rc --input $< --source pure64-data.c --header pure64-data.h --name pure64_data

stage-three-data.o: stage-three-data.c

stage-three-data.c stage-three-data.h: $(top)/src/stage-three/stage-three.sys rc
	./rc --input $< --source stage-three-data.c --header stage-three-data.h --name stage_three_data

%: %.o
	@echo "LD $@"
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	@echo "CC $@"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) mbr-data.h mbr-data.c mbr-data.o
	$(RM) pure64-data.h pure64-data.c pure64-data.o
	$(RM) stage-three-data.h stage-three-data.c stage-three-data.o
	$(RM) pure64.o pure64
	$(RM) rc.o rc

.PHONY: test
test:

install: pure64 $(DESTDIR)$(PREFIX)/bin
	cp pure64 $(DESTDIR)$(PREFIX)/bin/pure64

$(DESTDIR)$(PREFIX)/bin:
	mkdir -p $@

$(V).SILENT:
