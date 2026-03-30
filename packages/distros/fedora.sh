#!/bin/bash
sudo dnf -y install git cmake gdb gcc g++ make SDL2-devel vulkan-loader-devel execstack libcurl-devel
wget https://cdn.amazonlinux.com/al2023/core/guids/9cf1057036ef7d615de550a658447fad88617805da0cfc9b854ba0fb8a668466/x86_64/../../../../blobstore/218da5cb73cce7b5c684fef7eb03989f0c98601586fa105c5b254e59c8ea8e73/libGLEW-2.1.0-9.amzn2023.0.2.x86_64.rpm # fedora doesnt have libglew 2.1
wget https://cdn.amazonlinux.com/al2023/core/guids/9cf1057036ef7d615de550a658447fad88617805da0cfc9b854ba0fb8a668466/x86_64/../../../../blobstore/1f27b39ccf0300e88e092f06f28cb98e839929b4a842e98fdf7f4822f15d27b3/glew-devel-2.1.0-9.amzn2023.0.2.x86_64.rpm # fedora doesnt have glew-devel 2.1 anymore
sudo rpm -ivh libGLEW-2.1.0-9.amzn2023.0.2.x86_64.rpm
sudo rpm -ivh glew-devel-2.1.0-9.amzn2023.0.2.x86_64.rpm
rm libGLEW-2.1.0-9.amzn2023.0.2.x86_64.rpm
rm glew-devel-2.1.0-9.amzn2023.0.2.x86_64.rpm
sudo dnf -y group install "c-development"
sudo dnf -y group install "development-tools"
