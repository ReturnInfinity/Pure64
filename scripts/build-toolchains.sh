#!/bin/sh

export prefix=$PWD/toolchains

echo $prefix

export target=x86_64-none-elf
scripts/build-bintutils.sh
scripts/build-gcc.sh

export target=riscv64-none-elf
scripts/build-binutils.sh
scripts/build-gcc.sh
