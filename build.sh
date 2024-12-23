#!/usr/bin/env bash

mkdir -p bin

cd src

nasm -DBIOS=1 pure64.asm -o ../bin/pure64-bios.sys -l ../bin/pure64-bios-debug.txt
nasm -DUEFI=1 pure64.asm -o ../bin/pure64-uefi.sys -l ../bin/pure64-uefi-debug.txt

cd boot

nasm bios.asm -o ../../bin/bios.sys -l ../../bin/bios-debug.txt
nasm uefi.asm -o ../../bin/uefi.sys -l ../../bin/uefi-debug.txt
nasm bios-floppy.asm -o ../../bin/bios-floppy.sys -l ../../bin/bios-floppy-debug.txt
nasm bios-pxe.asm -o ../../bin/bios-pxe.sys -l ../../bin/bios-pxe-debug.txt

cd ../..
