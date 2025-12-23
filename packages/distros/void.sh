#!/bin/bash

sudo xbps-install make cmake gcc vulkan-loader-devel SDL SDL2-devel wget glew-devel elfutils-devel libselinux libselinux-devel gdb

wget -P libs/ https://downloads.sourceforge.net/glew/glew-2.1.0.tgz
cd libs/
tar xvf glew-2.1.0.tgz
cd glew-2.1.0
make -j$(nproc)

cd ../../

wget -P libs/ https://launchpadlibrarian.net/477189013/execstack_0.0.20131005-1.1_amd64.deb
cd libs/
ar -x execstack_0.0.20131005-1.1_amd64.deb
tar xvf data.tar.xz
sudo cp usr/bin/execstack /usr/bin/
