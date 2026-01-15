#!/bin/bash

# This script is intended to attach GDB and inject, and not much else.
# That means no automatic log display. Open a seperate shell and `tail -f /tmp/tf2.log` for active logs.

LIB_PATH=$(pwd)/tf2.so
PROCID=$(pgrep tf_linux64 | head -n 1)

if [[ "$(execstack -q tf2.so)" = "X tf2.so" ]]; then
    execstack -c tf2.so
fi

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

if [ -z "$PROCID" ]; then
    echo "Please open game"
    exit 1
fi

sudo gdb -n -q -ex "attach $PROCID" \
     -ex "call ((void * (*) (const char*, int)) dlopen)(\"$LIB_PATH\", 1)" \
     -ex "alias un = call call (int (*)(void*))dlclose($1)" \
     -ex "continue"

