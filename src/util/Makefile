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

pure64_files += pure64.o
pure64_files += stage-three-data.o
pure64_files += pure64-data.o
pure64_files += pxe-data.o
pure64_files += mbr-data.o
pure64_files += multiboot-data.o
pure64_files += multiboot2-data.o
pure64_files += fstream.o
pure64_files += util.o

generated_headers += stage-three-data.h
generated_headers += pure64-data.h
generated_headers += pxe-data.h
generated_headers += mbr-data.h
generated_headers += multiboot-data.h
generated_headers += multiboot2-data.h

.PHONY: all
all: $(targets)

rc: rc.o

rc.o: rc.c

pure64: $(pure64_files) -lpure64

pure64.o: pure64.c fstream.h util.h $(generated_headers)

util.o: util.c util.h fstream.h

fstream.o: fstream.c fstream.h

mbr-data.o: mbr-data.c

mbr-data.c mbr-data.h: $(top)/src/bootsectors/mbr.sys rc
	./rc --input $< --source mbr-data.c --header mbr-data.h --name mbr_data

multiboot-data.o: multiboot-data.c

multiboot-data.c multiboot-data.h: $(top)/src/bootsectors/multiboot.sys rc
	./rc --input $< --source multiboot-data.c --header multiboot-data.h --name multiboot_data

multiboot2-data.o: multiboot2-data.c

multiboot2-data.c multiboot2-data.h: $(top)/src/bootsectors/multiboot2.sys rc
	./rc --input $< --source multiboot2-data.c --header multiboot2-data.h --name multiboot2_data

pure64-data.o: pure64-data.c

pure64-data.c pure64-data.h: $(top)/src/pure64.sys rc
	./rc --input $< --source pure64-data.c --header pure64-data.h --name pure64_data

pxe-data.o: pxe-data.c

pxe-data.c pxe-data.h: $(top)/src/bootsectors/pxestart.sys rc
	./rc --input $< --source pxe-data.c --header pxe-data.h --name pxe_data

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
