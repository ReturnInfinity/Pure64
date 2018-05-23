#!/bin/bash

set -e
set -u

path="$1"

mkdir -p "$path"
mkdir -p "$path/bootsectors"

function install_file {
	src="${PWD}/$1"
	dst="$2/`basename $1`"
	if [ "$src" -nt "$dst" ]; then
		cp "${src}" "${dst}"
	fi
}

install_file "pure64.sys" "${path}"
install_file "bootsectors/mbr.sys" "${path}/bootsectors"
install_file "bootsectors/pxestart.sys" "${path}/bootsectors"
install_file "bootsectors/multiboot.sys" "${path}/bootsectors"
install_file "bootsectors/multiboot2.sys" "${path}/bootsectors"
