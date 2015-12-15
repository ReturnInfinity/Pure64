#!/bin/bash

nasm src/bootsectors/bmfs_mbr.asm -o bmfs_mbr.sys -l bmfs_mbr.lst
nasm src/bootsectors/pxestart.asm -o pxestart.sys -l pxestart.lst
cd src
nasm pure64.asm -o ../pure64.sys -l ../pure64.lst
cd ..
