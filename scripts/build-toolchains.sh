#!/bin/sh

export prefix=$PWD/toolchains

#if [ ! -e "$prefix/x86_64-none-elf" ]; then
	export target=x86_64-none-elf
	scripts/build-bintutils.sh
	scripts/build-gcc.sh
#fi

# Disabling for now
#export target=riscv64-none-elf
#scripts/build-binutils.sh
#scripts/build-gcc.sh
