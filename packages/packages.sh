#!/bin/bash
# Function to detect Linux distribution and install packages accordingly
{
  if [ -f /etc/arch-release ]; then
    echo "Detected Arch Linux or derivative"
    ./distros/arch.sh
  elif [ -f /etc/debian_version ]; then
    echo "Detected Debian/Ubuntu or derivative"
    ./distros/debian.sh
  elif [ -f /etc/redhat-release ]; then
    echo "Detected Fedora/CentOS"
    ./distros/fedora.sh
  else
    echo "Unsupported Linux distribution."
    exit 1
  fi
}
