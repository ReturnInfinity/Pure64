#!/bin/sh

# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS-Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Pass flags required for use in kernel environment
CFLAGS="$CFLAGS -fomit-frame-pointer -fno-stack-protector -mno-red-zone"
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Compiler is GNU
CC=gcc
# Build the object files
$CC $CFLAGS -c dap.c
$CC $CFLAGS -c dir.c
$CC $CFLAGS -c error.c
$CC $CFLAGS -c file.c
$CC $CFLAGS -c fs.c
$CC $CFLAGS -c mbr.c
$CC $CFLAGS -c misc.c
$CC $CFLAGS -c path.c
$CC $CFLAGS -c stream.c
$CC $CFLAGS -c string.c
$CC $CFLAGS -c uuid.c
# Use ar from binutils
AR=ar
# Flags for creating library
ARFLAGS=rcs
# Create library
$AR $ARFLAGS libpure64.a *.o
