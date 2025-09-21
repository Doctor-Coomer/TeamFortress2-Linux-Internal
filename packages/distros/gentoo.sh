#!/bin/bash

# Setup the overlay that has execstack
sudo eselect repository enable gentoo-zh
sudo emaint -r gentoo-zh sync
sudo bash -c "echo \"dev-util/execstack ~amd64\" > /etc/portage/package.accept_keywords/execstack"

# Build and install as many packages with emerge as possible
sudo emerge media-libs/glew media-libs/libsdl dev-debug/gdb dev-util/vulkan-headers dev-util/execstack 

# Build and install libGLEW.so.2.1
cd /tmp
wget https://versaweb.dl.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0.tgz?viasf=1
mkdir glew-2.1.0
tar -zxvf glew-2.1.0.tgz -C ./glew-2.1.0
cd glew-2.1.0/
make
sudo make install.all
