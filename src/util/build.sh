#!/bin/bash

source ../../bash/common.sh

compile_file pure64.c
compile_file fstream.c
compile_file memory.c
compile_file util.c

LDFLAGS="${LDFLAGS} -L ../core"
LDFLAGS="${LDFLAGS} -L ../lang"

LDLIBS="${LDLIBS} -lpure64-core"
LDLIBS="${LDLIBS} -lpure64-lang"

link_executable pure64 *.o
