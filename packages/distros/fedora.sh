#!/bin/bash
#bug: installs libGLEW 2.2 while building needs libGLEW 2.1
dnf -y install git cmake gdb gcc g++ make SDL2-devel glew-devel libGLEW
dnf -y group install "c-development"
dnf -y group install "development-tools"
