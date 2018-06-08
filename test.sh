#!/bin/bash

opts=`getopt -o hd:c: --long help,disk:,config: -n "Test Options" -- "$@"`
if [ $? != 0 ]; then
	echo "Failed to parse test options."
	exit 1
fi

eval set -- "${opts}"

function print_help {
	echo "Options:"
	echo "	-h, --help               : Print this help message and exit."
	echo "	-d, --disk          PATH : Specify the disk path."
	echo "	-c, --config        PATH : Specify the configuration file (used if disk does not exist.)"
	echo "	-r, --resource-path PATH : Specify the path to the boot loader resources (used if disk does not exist.)"
}

disk="pure64.img"
config="examples/example1-config.txt"
resources="out/share/pure64/arch"

while true; do
	case "$1" in
		-h | --help ) print_help; exit 0;;
		-d | --disk ) disk="$2"; shift 2;;
		-c | --config ) config="$2"; shift 2;;
		-r | --resource-path ) resources="$2"; shift 2;;
		-- ) shift; break;;
		* ) break;;
	esac
done

if [ ! -e "$disk" ]; then
	PURE64_RESOURCE_PATH="$resources" src/util/pure64 --disk "$disk" --config "examples/example1-config.txt" init
	if [ $? != 0 ]; then
		rm -f "$disk"
		exit 1
	fi
fi

set -u

# from http://unix.stackexchange.com/questions/9804/how-to-comment-multi-line-commands-in-shell-scripts

cmd=( qemu-system-x86_64
	-cpu core2duo
	-display sdl
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
