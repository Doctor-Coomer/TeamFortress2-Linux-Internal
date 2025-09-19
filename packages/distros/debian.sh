#!/bin/sh
sudo apt install cmake gdb gcc g++ make libsdl2-dev libglew-dev -y
wget https://archive.ubuntu.com/ubuntu/pool/universe/g/glew/libglew-dev_2.1.0-4_amd64.deb
wget https://archive.ubuntu.com/ubuntu/pool/universe/g/glew/libglew2.1_2.1.0-4_amd64.deb
sudo dpkg -i libglew-dev_2.1.0-4_amd64.deb
sudo dpkg -i libglew2.1_2.1.0-4_amd64.deb
sudo apt install libglew-dev -y #this may seem confusing
rm libglew-dev_2.1.0-4_amd64.deb libglew2.1_2.1.0-4_amd64.deb
