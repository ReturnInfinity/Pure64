#!/bin/bash

source "../../bash/common.sh"

if [ "$arch" == "" ]; then
	arch="x86-64"
fi

# Exit on error
set -e

# Tell GCC not to use the red zone
CFLAGS="${CFLAGS} -mno-red-zone"
# No frame pointer
CFLAGS="${CFLAGS} -fomit-frame-pointer"
# No stack protector, libssp not available
CFLAGS="${CFLAGS} -fno-stack-protector"

# Build the object files
compile_file _start.c
compile_file ahci.c
compile_file debug.c
compile_file e820.c
compile_file hooks.c
compile_file map.c
compile_file pci.c
# Use 'ld' instead of GCC for linker
LD=${CROSS_COMPILE}ld
# Pass linker script
LDFLAGS="${LDFLAGS} -T loader-${arch}.ld"
LDFLAGS="${LDFLAGS} -L ../core"
# Pass Pure64 library
LDLIBS="${LDLIBS} -lpure64-core"
# Create the file system loader
link_executable "fs-loader" *.o
# Convert it to a flat binary, fs-loader.bin
convert_to_binary "fs-loader"
