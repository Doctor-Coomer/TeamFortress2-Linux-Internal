#!/bin/bash
# Function to detect Linux distribution and install packages accordingly
{
  if [ -f /etc/arch-release ]; then
    echo "Detected Arch Linux or derivative"
    sudo pacman -S base-devel cmake sdl2 gdb glew

    cd /tmp
    git clone https://aur.archlinux.org/glew-2.1.git
    cd glew-2.1
    makepkg -is

    cd /tmp
    git clone https://aur.archlinux.org/proggyfonts.git
    cd proggyfonts
    makepkg -is

    rm -rf /tmp/glew-2.1
    rm -rf /tmp/proggyfonts
  elif [ -f /etc/debian_version ]; then
    echo "Detected Debian/Ubuntu or derivative"
    sudo apt install cmake gdb gcc g++ make libsdl2-dev libglew-dev -y
    wget https://archive.ubuntu.com/ubuntu/pool/universe/g/glew/libglew-dev_2.1.0-4_amd64.deb
    wget https://archive.ubuntu.com/ubuntu/pool/universe/g/glew/libglew2.1_2.1.0-4_amd64.deb
    sudo dpkg -i libglew-dev_2.1.0-4_amd64.deb
    sudo dpkg -i libglew2.1_2.1.0-4_amd64.deb
    sudo apt install libglew-dev -y #this may seem confusing
    rm libglew-dev_2.1.0-4_amd64.deb libglew2.1_2.1.0-4_amd64.deb

  #support for more distros can be added
  #elif [ -f /etc/redhat-release ]; then
    #echo "Detected RHEL derivative"
    #
  #elif [[ "$(lsb_release -si)" == "openSUSE" ]]; then
    #echo "Detected openSUSE"
    #

  else
    echo "Unsupported Linux distribution."
    exit 1
  fi
}
