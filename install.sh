#!/bin/bash

opts=`getopt -o ha:b:i:u:p: --long help,architecture:,install-bootloader:,install-headers:,install-utility:,prefix: -n "Install Options" -- "$@"`
if [ $? != 0 ]; then
	echo "Failed to parse install options."
	exit 1
fi

eval set -- "${opts}"

set -e

function print_help {
	echo "Options:"
	echo "	-h, --help                      : Print this help message and exit."
	echo "	-a, --architecture        ARCH  : Specify architecture to install."
	echo "	-b, --install-bootloader  BOOL  : Install the bootloader."
	echo "	-l, --install-libraries   BOOL  : Install the libraries."
	echo "	-i, --install-headers     BOOL  : Install the headers."
	echo "	-u, --install-utility     BOOL  : Install the utility program."
	echo "	-p, --prefix              PATH  : Specify the path prefix for the install."
	echo "Values:"
	echo "	ARCH  : Can either be 'x86_64' or 'riscv64'"
	echo "	BOOL  : Can either be 'yes' or 'no'"
	echo "	CROSS : A cross compiler prefix, without the trailing '-'"
	echo "	PATH  : An absolute path."
}

function install_dir {
	echo "Entering $1"
	original_dir="${PWD}"
	cd "$1"
	./install.sh "$2"
	cd "${original_dir}"
}

arch="x86_64"
install_bootloader="yes"
install_headers="yes"
install_libraries="yes"
install_utility="yes"
prefix="/usr/local"

while true; do
	case "$1" in
		-h | --help ) print_help; exit 0;;
		-a | --architecture ) arch="$2"; shift 2;;
		-b | --install-bootloader ) install_bootloader="$2"; shift 2;;
		-i | --install-headers ) install_headers="$2"; shift 2;;
		-l | --install-libraries ) install_libraries="$2"; shift 2;;
		-u | --install-utility ) install_utility="$2"; shift 2;;
		-p | --prefix ) prefix="$2";  shift 2;;
		-- ) shift; break;;
		* ) break;;
	esac
done

if [ "$install_bootloader" == "yes" ]; then
	install_dir "src/arch/${arch}" "${prefix}/share/pure64/arch/${arch}"
fi

if [ "$install_headers" == "yes" ]; then
	mkdir -p "${prefix}/include/pure64"
	mkdir -p "${prefix}/include/pure64/core"
	mkdir -p "${prefix}/include/pure64/lang"
	cp "include/pure64/core.h" "${prefix}/include/pure64"
	cp "include/pure64/lang.h" "${prefix}/include/pure64"
	cp "include/pure64/core/"*.h "${prefix}/include/pure64/core"
	cp "include/pure64/lang/"*.h "${prefix}/include/pure64/lang"
fi

if [ "$install_libraries" == "yes" ]; then
	mkdir -p "${prefix}/lib"
	cp "src/core/libpure64-core.a" "${prefix}/lib"
	cp "src/lang/libpure64-lang.a" "${prefix}/lib"
fi

if [ "$install_utility" == "yes" ]; then
	mkdir -p "${prefix}/bin"
	cp "src/util/pure64" "${prefix}/bin"
fi
