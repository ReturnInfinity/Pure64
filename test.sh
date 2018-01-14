#!/bin/bash

# from http://unix.stackexchange.com/questions/9804/how-to-comment-multi-line-commands-in-shell-scripts

cmd=( qemu-system-x86_64
# Window title in graphics mode
	-name "Pure64 Test"
# Amount of CPU cores
	-smp 2
# Amount of memory in Megabytes
	-m 256
# Disk configuration
	-drive id=disk,file="pure64.img",if=none,format=raw
	-device ahci,id=ahci
	-device ide-drive,drive=disk,bus=ahci.0
# Enable GDB debugging
	-s
)

#execute the cmd string
"${cmd[@]}"
