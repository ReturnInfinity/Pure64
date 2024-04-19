#!/bin/bash

mkdir -p bin

cd src

nasm pure64.asm -o ../bin/pure64.sys -l ../bin/pure64-debug.txt

cd boot

nasm bios.asm -o ../../bin/bios.sys -l ../../bin/bios-debug.txt
nasm uefi.asm -o ../../bin/uefi.sys -l ../../bin/uefi-debug.txt

cd ../..
