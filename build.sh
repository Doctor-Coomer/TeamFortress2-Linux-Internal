#!/bin/bash

PROJECT_ROOT=$(pwd)

# Make a directory for our binary libraries
if [ ! -d "libs/" ]; then
    mkdir libs
fi

# Clone and build funchook, if not done already.
if [ -d "libs/funchook" ] && [ ! -f "libs/funchook/build/libfunchook.a" ] && [ ! -f "libs/funchook/build/libdistorm.a" ]; then
    rm -rf libs
fi

if [ ! -d "libs/funchook" ]; then
    mkdir libs/funchook
    git clone https://github.com/Doctor-Coomer/funchook.git libs/funchook/
    mkdir libs/funchook/build
    cd libs/funchook
    cmake -DCMAKE_BUILD_TYPE=Release .
    make
fi

# Build hack
cd $PROJECT_ROOT
make clean && make
if [ -f "tf2.so" ]; then
    if [[ "$(execstack -q tf2.so)" = "X tf2.so" ]]; then
	execstack -c tf2.so
    fi
fi
