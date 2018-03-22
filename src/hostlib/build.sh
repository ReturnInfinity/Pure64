#!/bin/sh

# Set source files path
srcdir=${PWD}/../lib/
# Set compiler flags
CFLAGS="$CFLAGS -O2"
# Include the build script
. ../lib/build.sh
