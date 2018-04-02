#!/bin/bash

set -e

cd src/lib && ./build.sh && cd ../..
cd src/util && ./build.sh && cd ../..
