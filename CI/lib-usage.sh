#!/bin/bash

# Play it safe: exit if something fails
set -e -o pipefail -o errtrace -o functrace

# Install libhpack
make all doc -j
sudo make -C build install

# Compile tests
HPACK_CFLAGS=`pkg-config --cflags libhpack`
HPACK_LIBS=`pkg-config --libs libhpack`

cd CI
$CC -c -o lib-usage-1.o lib-usage-1.c ${HPACK_CFLAGS}
$CC -o lib-usage-1 lib-usage-1.o ${HPACK_LIBS}
cd ..
