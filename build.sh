#!/bin/bash

#replace Arch with distro name
distroname=$(lsb_release -is)
sed -i 's/Arch/'${distroname}'/g' hooks/sdl.cpp

# Clone and build funchook
if [ -d "libs/funchook" ]; then
    rm -rf libs
fi

if [ ! -d "libs/funchook" ]; then
    mkdir libs
    cd libs
    git clone https://github.com/Doctor-Coomer/funchook.git
    cd funchook
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
fi

# Escape back to the root project directory
cd ../../..

# Build hack
make clean && make
