#!/bin/bash

sudo pacman -S base-devel cmake sdl2 gdb glew vulkan-devel

cd /tmp
git clone https://aur.archlinux.org/glew-2.1.git
cd glew-2.1
makepkg -is

cd /tmp
git clone https://aur.archlinux.org/execstack.git
cd execstack
makepkg -is

cd /tmp
git clone https://aur.archlinux.org/proggyfonts.git
cd proggyfonts
makepkg -is

rm -rf /tmp/glew-2.1
rm -rf /tmp/proggyfonts
rm -rf /tmp/execstack
