#!/bin/bash

ver="2.30"
url="https://ftp.gnu.org/gnu/binutils/binutils-$ver.tar.gz"

if [ ! -e "binutils-$ver.tar.gz" ]; then
	wget $url
fi

if [ ! -e "binutils-$ver" ]; then
	tar --extract --file "binutils-$ver.tar.gz"
fi

builddir=binutils-$ver/build/$target

mkdir -p $builddir
cd $builddir
../../configure --target $target --prefix=$prefix --with-sysroot --disable-nls --disable-werror
make
make install
