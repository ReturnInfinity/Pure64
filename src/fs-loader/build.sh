#!/bin/sh

# Exit on error
set -e
# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS-Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Pass flags required for use in kernel environment
CFLAGS="$CFLAGS -ffreestanding"
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Set the cross compiler prefix
CROSS_COMPILE=x86_64-none-elf-
# Set the compiler name
CC=${CROSS_COMPILE}gcc
# Build the object files
$CC $CFLAGS -c _start.c
$CC $CFLAGS -c ahci.c
$CC $CFLAGS -c debug.c
$CC $CFLAGS -c e820.c
$CC $CFLAGS -c hooks.c
$CC $CFLAGS -c map.c
$CC $CFLAGS -c pci.c
# Set the linker name
LD=${CROSS_COMPILE}ld
# Pass linker script
LDFLAGS="$LDFLAGS -T stage-three.ld -nostdlib"
# Pass library search directroy
LDFLAGS="$LDFLAGS -L ../targetlib"
# Pass Pure64 library
LDLIBS="$LDLIBS -lpure64"
# Create the stage-three loader
$LD $LDFLAGS *.o -o stage-three $LDLIBS
# Set the objcopy name
OBJCOPY=${CROSS_COMPILE}objcopy
# Convert the loader into a flat binary
$OBJCOPY -O binary stage-three stage-three.sys
