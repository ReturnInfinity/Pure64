#!/bin/sh

nasm mbr.asm -o mbr.sys
nasm pxestart.asm -o pxestart.sys
nasm multiboot.asm -o multiboot.sys
nasm multiboot2.asm -o multiboot2.sys
