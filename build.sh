#!/bin/bash

# Clone and build funchook, if not done already.
if [ -d "libs/funchook" ] && [ ! -f "libs/funchook/build/libfunchook.a" ] && [ ! -f "libs/funchook/build/libdistorm.a" ]; then
    rm -rf libs
fi

if [ ! -d "libs/funchook" ]; then
    mkdir libs
    git clone https://github.com/Doctor-Coomer/funchook.git libs/
    mkdir libs/funchook/build
    cmake -DCMAKE_BUILD_TYPE=Release libs/funchook/build
    make
fi

# Build hack
make clean && make
