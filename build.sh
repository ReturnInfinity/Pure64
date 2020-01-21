#!/bin/bash

mkdir -p bin

cd src

nasm pure64.asm -o ../bin/pure64.sys

cd bootsectors

nasm mbr.asm -o ../../bin/mbr.sys
nasm pxestart.asm -o ../../bin/pxestart.sys
nasm multiboot.asm -o ../../bin/multiboot.sys
nasm multiboot2.asm -o ../../bin/multiboot2.sys

cd ../..
