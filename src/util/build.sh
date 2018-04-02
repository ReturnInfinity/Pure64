#!/bin/sh

# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS-Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Build the utility program
gcc $CFLAGS pure64.c fstream.c memory.c util.c config.c token.c -o pure64 ../lib/libpure64.a
