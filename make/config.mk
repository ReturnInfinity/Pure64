ifndef TOP
$(error "TOP variable must be defined")
endif
DESTDIR ?=
PREFIX ?= /opt/return-infinity
ARCH ?= x86_64
ARCH_$(ARCH) = 1
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NASM = nasm
CFLAGS += -Wall -Wextra -Werror -Wfatal-errors -std=gnu99
CFLAGS += -I $(TOP)/include
