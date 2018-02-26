#!/bin/sh

# Exit on error
set -e
# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS-Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Pass flags required for use in kernel environment
CFLAGS="$CFLAGS -fomit-frame-pointer -fno-stack-protector -mno-red-zone"
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Build the object files
gcc $CFLAGS -c _start.c
gcc $CFLAGS -c ahci.c
gcc $CFLAGS -c debug.c
gcc $CFLAGS -c e820.c
gcc $CFLAGS -c hooks.c
gcc $CFLAGS -c map.c
gcc $CFLAGS -c pci.c
# Pass linker script
LDFLAGS="$LDFLAGS -T stage-three.ld"
# Pass library search directroy
LDFLAGS="$LDFLAGS -L ../lib"
# Pass Pure64 library
LDLIBS="$LDLIBS -lpure64"
# Create the stage-three loader
ld $LDFLAGS *.o -o stage-three $LDLIBS
# Convert the loader into a flat binary
objcopy -O binary stage-three stage-three.sys
