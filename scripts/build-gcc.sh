#!/bin/bash

ver="7.3.0"
url="https://ftp.gnu.org/gnu/gcc/gcc-$ver/gcc-$ver.tar.gz"

if [ ! -e "gcc-$ver.tar.gz" ]; then
	wget $url
fi

if [ ! -e "gcc-$ver" ]; then
	tar --extract --file "gcc-$ver.tar.gz"
fi

builddir=gcc-$ver/build/$target

mkdir -p $builddir
cd $builddir
../../configure --prefix "$prefix" --target $target --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
