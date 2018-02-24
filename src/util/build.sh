#!/bin/sh

# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS-Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Build the resource compiler
gcc $CFLAGS rc.c -o rc
# Generate the MBR source
./rc --input ../bootsectors/mbr.sys --source mbr-data.c --header mbr-data.h --name mbr_data
# Generate the 2nd stage boot loader source
./rc --input ../pure64.sys --source pure64-data.c --header pure64-data.h --name pure64_data
# Generate the 3rd stage boot loader source
./rc --input ../stage-three/stage-three.sys --source stage-three-data.c --header stage-three-data.h --name stage_three_data
# Build the utility program
gcc $CFLAGS pure64.c mbr-data.c pure64-data.c stage-three-data.c -o pure64 ../lib/libpure64.a
