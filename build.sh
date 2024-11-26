#!/bin/bash

mkdir -p bin

cd src

export BIOS=1
nasm pure64.asm -o ../bin/pure64-bios.sys -l ../bin/pure64-bios-debug.txt
unset BIOS
export UEFI=1
nasm pure64.asm -o ../bin/pure64-uefi.sys -l ../bin/pure64-uefi-debug.txt
unset UEFI

cd boot

nasm bios.asm -o ../../bin/bios.sys -l ../../bin/bios-debug.txt
nasm uefi.asm -o ../../bin/uefi.sys -l ../../bin/uefi-debug.txt
nasm bios-floppy.asm -o ../../bin/bios-floppy.sys -l ../../bin/bios-floppy-debug.txt
nasm bios-pxe.asm -o ../../bin/bios-pxe.sys -l ../../bin/bios-pxe-debug.txt

cd ../..
