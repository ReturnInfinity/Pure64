#!/bin/sh

export prefix=~/.local

export target=x86_64-none-elf
scripts/build-bintutils.sh
scripts/build-gcc.sh

export target=riscv64-none-elf
scripts/build-binutils.sh
scripts/build-gcc.sh
