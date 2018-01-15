#!/bin/bash

set -e

cd src

# Build bootsectors
nasm bootsectors/bmfs_mbr.asm  -o bootsectors/bmfs_mbr.sys
nasm bootsectors/multiboot.asm -o bootsectors/multiboot.sys
nasm bootsectors/pxestart.asm  -o bootsectors/pxestart.sys

# Build pure64.sys
nasm pure64.asm -f elf64 -F dwarf -o pure64.o
gcc -c load.c -o load.o -I ../include -fno-stack-protector
ld pure64.o load.o -o pure64 -T pure64.ld
objcopy -O binary pure64 pure64.sys

# Build libpure64.a
gcc -c lib/dir.c  -o lib/dir.o  -I ../include
gcc -c lib/file.c -o lib/file.o -I ../include
gcc -c lib/misc.c -o lib/misc.o -I ../include
gcc -c lib/path.c -o lib/path.o -I ../include
ar rcs lib/libpure64.a lib/dir.o lib/file.o lib/misc.o lib/path.o

# Build util/rc
gcc util/rc.c -o util/rc

# Generate util/pure64-data.c
util/rc --input pure64.sys \
        --source util/pure64-data.c \
        --header util/pure64-data.h \
        --name pure64_data

# Generate util/mbr-data.c
util/rc --input bootsectors/bmfs_mbr.sys \
        --source util/mbr-data.c \
        --header util/mbr-data.h \
        --name mbr_data

# Build util/pure64
gcc -o util/pure64 \
       util/pure64.c \
       util/pure64-data.c \
       util/mbr-data.c \
       lib/libpure64.a \
       -I ../include -iquote .

cd ..
