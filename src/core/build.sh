#!/bin/bash

source "../../bash/common.sh"

CFLAGS="${CFLAGS} -mno-red-zone"
CFLAGS="${CFLAGS} -fno-stack-protector"
CFLAGS="${CFLAGS} -fomit-frame-pointer"

set -e

compile_file dap.c
compile_file dir.c
compile_file error.c
compile_file file.c
compile_file fs.c
compile_file gpt.c
compile_file mbr.c
compile_file misc.c
compile_file partition.c
compile_file path.c
compile_file stream.c
compile_file string.c
compile_file uuid.c

link_static libpure64-core.a *.o
