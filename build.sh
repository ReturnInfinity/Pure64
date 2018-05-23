#!/bin/bash

opts=`getopt -o ha:b:u:x: --long help,architecture,build-bootloader,build-utility,cross-compile: -n "Build Options" -- "$@"`
if [ $? != 0 ]; then
	echo "Failed to parse build options."
	exit 1
fi

eval set -- "${opts}"

set -e

function print_help {
	echo "Options:"
	echo "	-h, --help                    : Print this help message and exit."
	echo "	-a, --architecture      ARCH  : Specify architecture to build for."
	echo "	-b, --build-bootloader  BOOL  : Build the bootloader."
	echo "	-l, --build-libraries   BOOL  : Build the libraries."
	echo "	-u, --build-utility     BOOL  : Build the utility program."
	echo "	-x, --cross-compile     CROSS : Specify cross compiler prefix."
	echo "Values:"
	echo "	ARCH  : Can either be 'x86_64' or 'riscv64'"
	echo "	BOOL  : Can either be 'yes' or 'no'"
	echo "	CROSS : A cross compiler prefix, without the trailing '-'"
}

function set_cross_compiler {
	CROSS_COMPILE="$2"
	export CC="${CROSS_COMPILE}${CC}"
	export AR="${CROSS_COMPILE}${AR}"
	export AS="${CROSS_COMPILE}${AS}"
	export LD="${CROSS_COMPILE}${LD}"
}

function build_dir {
	echo "Entering $1"
	original_dir="${PWD}"
	cd "$1"
	./build.sh
	cd "${original_dir}"
}

ARCH="x86_64"
build_bootloader="yes"
build_libraries="yes"
build_utility="yes"

while true; do
	case "$1" in
		-h | --help ) print_help; exit 0;;
		-a | --architecture ) ARCH="$2"; shift 2;;
		-b | --build-bootloader ) build_bootloader="$2"; shift 2;;
		-l | --build-libraries ) build_libraries="$2"; shift 2;;
		-u | --build-utility ) build_utility="$2"; shift 2;;
		-x | --cross-compile ) set_cross_compiler "$2";  shift 2;;
		-- ) shift; break;;
		* ) break;;
	esac
done

if [ "$build_bootloader" == "yes" ]; then
	build_dir "src/arch/${ARCH}"
fi

if [ "$build_libraries" == "yes" ]; then
	build_dir "src/core"
	build_dir "src/lang"
fi

if [ "$build_utility" == "yes" ]; then
	build_dir "src/util"
fi
