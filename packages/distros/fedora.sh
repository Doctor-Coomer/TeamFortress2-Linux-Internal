#!/bin/bash
sudo dnf -y install git cmake gdb gcc g++ make SDL2-devel glew-devel vulkan-loader-devel execstack
wget https://ftp.lysator.liu.se/pub/opensuse/distribution/leap/15.6/repo/oss/x86_64/libGLEW2_1-2.1.0-1.28.x86_64.rpm #fedora doesnt have libglew 2.1
sudo rpm -ivh libGLEW2_1-2.1.0-1.28.x86_64.rpm
rm libGLEW2_1-2.1.0-1.28.x86_64.rpm
sudo dnf -y group install "c-development"
sudo dnf -y group install "development-tools"
