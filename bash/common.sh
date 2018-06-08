#!/bin/bash

# Set the compiler to GCC
CC=${CROSS_COMPILE}gcc
# Enable all compiler warnings, treat warnings as errors
CFLAGS="$CFLAGS -Wall -Wextra -Werror -Wfatal-errors"
# Enable C11 standard with GNU extensions
CFLAGS="$CFLAGS -std=gnu11"
# Generate debugging information
CFLAGS="$CFLAGS -g"
# Pass the include directory
CFLAGS="$CFLAGS -I ../../include"
# Let the NASM binary be found via the PATH environment variable.
NASM=nasm
# No flags currently defined for NASM
NASMFLAGS=

function assemble_binary {
	src=${srcdir}$1
	out=${objdir}`basename -s .asm $1`.sys
	echo " NASM    $PWD/$out"
	$NASM $NASMFLAGS $src -o $out
}

function compile_file {
	src=${srcdir}$1
	obj=${objdir}`basename -s .c $1`.o
	echo " CC      $PWD/$obj"
	$CC $CFLAGS -c $src -o $obj
}

# Use ar from binutils
AR=${CROSS_COMPILE}ar
# Flags for creating library
ARFLAGS=rcs

function link_static {
	echo " AR      $PWD/$1"
	$AR $ARFLAGS $1 ${@:2}
}

# Use GCC for linking
LD=${CROSS_COMPILE}gcc
# No linker flags currently
LDFLAGS=
# No default libs
LDLIBS=

function link_executable {
	echo " LD      $PWD/$1"
	$LD $LDFLAGS ${@:2} -o $1 $LDLIBS
}

# Use binutils' objcopy to generate flat binary
OBJCOPY=${CROSS_COMPILE}objcopy

function convert_to_binary {
	src="$1"
	out=`basename "$src"`.sys
	echo " OBJCOPY $PWD/$out"
	$OBJCOPY -O binary "$src" "$out"
}
