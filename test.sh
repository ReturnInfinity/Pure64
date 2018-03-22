#!/bin/bash

# from http://unix.stackexchange.com/questions/9804/how-to-comment-multi-line-commands-in-shell-scripts

set -e

if [ "$1" == "" ]; then
	disk="pure64.img"
else
	disk=$1
fi

set -u

cmd=( qemu-system-x86_64
	-cpu core2duo
	-display none
	-serial stdio
# Window title in graphics mode
	-name "Pure64 Test"
# Amount of CPU cores
	-smp 2
# Amount of memory in Megabytes
	-m 256
# Disk configuration
	-drive id=disk,file="$disk",if=none,format=raw
	-device ahci,id=ahci
	-device ide-drive,drive=disk,bus=ahci.0
# Enable GDB debugging
#	-s
#	-S
)

#execute the cmd string
"${cmd[@]}"
