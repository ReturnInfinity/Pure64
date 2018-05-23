#!/bin/bash

source ../../bash/common.sh

compile_file config.c
compile_file parser.c
compile_file scanner.c
compile_file token.c
compile_file var.c

link_static libpure64-lang.a *.o
