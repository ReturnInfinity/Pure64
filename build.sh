#!/bin/bash

set -e

cd src/bootsectors && ./build.sh && cd ../..
cd src && ./build.sh && cd ..
cd src/targetlib && ./build.sh && cd ../..
cd src/hostlib && ./build.sh && cd ../..
cd src/stage-three && ./build.sh && cd ../..
cd src/util && ./build.sh && cd ../..
