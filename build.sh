#!/bin/bash

set -e

function build_dir {
	echo "Entering ${PWD}/$1"
	cd $1
	./build.sh
	cd ../..
}

build_dir "src/core"
build_dir "src/lang"
build_dir "src/util"
