#!/bin/bash

source "../../../bash/common.sh"

assemble_binary "pure64.asm"

cd "bootsectors"
assemble_binary "mbr.asm"
assemble_binary "pxestart.asm"
assemble_binary "multiboot.asm"
assemble_binary "multiboot2.asm"
cd ".."
