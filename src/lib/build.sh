#!/bin/sh

# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS -Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Pass flags required for use in kernel environment
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Compiler is GNU
CC=${CROSS_COMPILE}gcc
# Build the object files
$CC $CFLAGS -c ${srcdir}dap.c
$CC $CFLAGS -c ${srcdir}dir.c
$CC $CFLAGS -c ${srcdir}error.c
$CC $CFLAGS -c ${srcdir}file.c
$CC $CFLAGS -c ${srcdir}fs.c
$CC $CFLAGS -c ${srcdir}gpt.c
$CC $CFLAGS -c ${srcdir}mbr.c
$CC $CFLAGS -c ${srcdir}misc.c
$CC $CFLAGS -c ${srcdir}partition.c
$CC $CFLAGS -c ${srcdir}path.c
$CC $CFLAGS -c ${srcdir}stream.c
$CC $CFLAGS -c ${srcdir}string.c
$CC $CFLAGS -c ${srcdir}uuid.c
# Use ar from binutils
AR=ar
# Flags for creating library
ARFLAGS=rcs
# Create library
$AR $ARFLAGS libpure64.a *.o
