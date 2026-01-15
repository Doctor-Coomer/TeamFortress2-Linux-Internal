#!/bin/bash

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

unload() {
    echo -e "\nUnloading library with handle $LIB_HANDLE"
    
    RC=$(sudo gdb -n --batch -ex "attach $PROCID" \
             -ex "call ((int (*) (void *)) dlclose)((void *) $LIB_HANDLE)" \
             -ex "call ((char * (*) (void)) dlerror)()" \
             -ex "detach" 2> /dev/null | grep -oP '\$2 = 0x\K[0-9a-f]+')

    if [[ "$RC" == "0" ]]; then
        echo "Library unloaded successfully"
    else
        echo "Failed to unload library"
    fi
}

trap unload SIGINT

LIB_HANDLE=$(sudo gdb -n --batch -ex "attach $PROCID" \
                  -ex "call ((void * (*) (const char*, int)) dlopen)(\"$LIB_PATH\", 1)" \
                  -ex "detach" 2> /dev/null | grep -oP '\$1 = \(void \*\) \K0x[0-9a-f]+'
	  )

if [ -z "$LIB_HANDLE" ]; then
    echo "Failed to load library"
    exit 1
fi

if [[ "$LIB_HANDLE" = "0x0" ]]; then
    echo "Failed to load library at $LIB_HANDLE"
    ERR=$(sudo gdb -n --batch -ex "attach $PROCID" \
	       -ex "call ((char * (*) (void)) dlerror)()" \
               -ex "detach" 2> /dev/null | grep '\$1')
    echo "Result from dlerror: $ERR"
    exit 1
fi

echo "Library loaded successfully at $LIB_HANDLE. Use Ctrl+C to unload."

tail -f /tmp/tf2.log
