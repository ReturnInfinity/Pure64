#!/bin/sh

export prefix=$PWD/toolchains

export target=x86_64-none-elf
scripts/build-bintutils.sh
scripts/build-gcc.sh

# Disabling for now
#export target=riscv64-none-elf
#scripts/build-binutils.sh
#scripts/build-gcc.sh
