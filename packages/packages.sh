#!/bin/bash
# Function to detect Linux distribution and install packages accordingly
{
    if [ -f /etc/arch-release ]; then
	echo "Detected Arch Linux or derivative"
	./packages/distros/arch.sh
    elif [ -f /etc/debian_version ]; then
	echo "Detected Debian/Ubuntu or derivative"
	./packages/distros/debian.sh
    elif [ -f /etc/redhat-release ]; then
	echo "Detected Fedora/CentOS"
	./packages/distros/fedora.sh
    elif [ -f /etc/gentoo-release ]; then
	echo "Detected Gentoo"
	./packages/distros/gentoo.sh
    else
	echo "Unsupported Linux distribution."
	exit 1
    fi
}
