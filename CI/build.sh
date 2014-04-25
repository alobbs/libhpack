#!/bin/bash

# Play it safe: exit if something fails
set -x -e -o pipefail -o errtrace -o functrace

# Build
mkdir -p build
cd build
cmake $CMAKE_OPTION -DCMAKE_BUILD_TYPE:STRING=Debug ..
make VERBOSE=1 $MAKE_OPTION
make $MAKE_OPTION doc

# Run tests
../tools/highlight-ctest.py "make test ARGS='-V'"
cd ..
