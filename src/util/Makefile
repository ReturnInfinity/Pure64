TOP ?= ../..
PREFIX ?= /opt/return-infinity
VPATH += $(TOP)/src/lib

include $(TOP)/make/patterns.mk

CC = $(HOST_CROSS_COMPILE)gcc

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CFLAGS += -std=gnu99
CFLAGS += -I $(TOP)/include
CFLAGS += -g

targets += pure64$(EXE)
targets += config-test$(EXE)
targets += token-test$(EXE)

pure64_files += pure64.o
pure64_files += config.o
pure64_files += fstream.o
pure64_files += memory.o
pure64_files += token.o
pure64_files += util.o

install_dir ?= $(DESTDIR)$(PREFIX)/bin

.PHONY: all
all: $(targets)

pure64$(EXE): $(pure64_files) -lpure64

pure64.o: pure64.c fstream.h util.h

config.o: config.c config.h

config-test.o: config-test.c config.h

config-test$(EXE): config-test.o config.o token.o $(bootsector_files)

fstream.o: fstream.c fstream.h

memory.o: memory.c

token.o: token.c token.h

token-test.o: token-test.c token.h

token-test$(EXE): token-test.o token.o

util.o: util.c util.h fstream.h

.PHONY: clean
clean:
	$(RM) pure64 $(pure64_files)
	$(RM) config-test config-test.o
	$(RM) token-test token-test.o

.PHONY: test
test: config-test token-test
	./token-test
	./config-test

install: all
	mkdir -p $(install_dir)
	cp pure64 $(install_dir)

$(V).SILENT:
