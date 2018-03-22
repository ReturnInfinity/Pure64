#!/bin/sh

# Set the directory that the
# source files can be found at.
srcdir=${PWD}/../lib/
# Set the cross compiler prefix
CROSS_COMPILE=x86_64-none-elf-
# Set the compiler flags
CFLAGS="$CFLAGS -ffreestanding -Os"
# Include the build source
. ../lib/build.sh
