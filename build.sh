#!/bin/bash

nasm src/bootsectors/bmfs_mbr.asm -o bmfs_mbr.sys
nasm src/bootsectors/multiboot.asm -o multiboot.sys
nasm src/bootsectors/pxestart.asm -o pxestart.sys
nasm src/pure64.asm -o pure64.sys

exit 0
