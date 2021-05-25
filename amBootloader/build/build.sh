#!/bin/sh

if [ -f Makefile ]; then
    echo "Found Makefile, skipping cmake..."
else
    echo "Cannot find makefile, running cmake..."
    cmake -DCMAKE_TOOLCHAIN_FILE="armgcc.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release  .
fi

make -j3
