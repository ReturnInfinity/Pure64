#!/bin/bash

function clean_dir {
	cd "$1"
	./clean.sh
	cd "../.."
}

clean_dir "src/core"
clean_dir "src/lang"
clean_dir "src/util"
