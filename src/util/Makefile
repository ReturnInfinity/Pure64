top ?= ../..

DESTDIR ?=

PREFIX ?= /usr/local

VPATH += $(top)/src/hostlib

CC = $(HOST_CROSS_COMPILE)gcc

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(top)/include
CFLAGS += -g

targets += rc
targets += pure64$(EXE)
targets += config-test$(EXE)
targets += token-test$(EXE)

pure64_files += pure64.o

# Generated sources
bootsector_files += mbr-data.o
bootsector_files += multiboot-data.o
bootsector_files += multiboot2-data.o
bootsector_files += pxe-data.o

# Generated sources
pure64_files += stage-three-data.o
pure64_files += pure64-data.o

pure64_files += config.o
pure64_files += fstream.o
pure64_files += memory.o
pure64_files += token.o
pure64_files += util.o
pure64_files += $(bootsector_files)

generated_headers += mbr-data.h
generated_headers += pure64-data.h
generated_headers += pxe-data.h
generated_headers += multiboot-data.h
generated_headers += multiboot2-data.h
generated_headers += stage-three-data.h

.PHONY: all
all: $(targets)

rc: rc.o
	gcc $< -o $@

rc.o: rc.c
	gcc -Wall -Wextra -Werror -Wfatal-errors -std=gnu99 -c $< -o $@

pure64$(EXE): $(pure64_files) -lpure64

pure64.o: pure64.c fstream.h util.h $(generated_headers)

config.o: config.c config.h

config-test.o: config-test.c config.h

config-test$(EXE): config-test.o config.o token.o $(bootsector_files)

fstream.o: fstream.c fstream.h

memory.o: memory.c

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

token.o: token.c token.h

token-test.o: token-test.c token.h

token-test$(EXE): token-test.o token.o

util.o: util.c util.h fstream.h

%.exe: %.o
	@echo "LD      $@"
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%: %.o
	@echo "LD      $@"
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	@echo "CC      $@"
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) pure64 $(pure64_files)
	$(RM) config-test config-test.o
	$(RM) token-test token-test.o
	$(RM) rc rc.o

.PHONY: test
test: config-test
	./token-test
	./config-test

install: pure64 $(DESTDIR)$(PREFIX)/bin
	cp pure64 $(DESTDIR)$(PREFIX)/bin/pure64

$(DESTDIR)$(PREFIX)/bin:
	mkdir -p $@

$(V).SILENT:
