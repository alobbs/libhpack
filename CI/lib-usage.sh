#!/bin/bash

# Play it safe: exit if something fails
set -x -e -o pipefail -o errtrace -o functrace

# Install libhpack
make all doc
sudo make -C build install

# Compile tests
HPACK_CFLAGS=`pkg-config --cflags libhpack`
HPACK_LIBS=`pkg-config --libs libhpack`
CHULA_CFLAGS=`pkg-config --cflags libchula`
CHULA_LIBS=`pkg-config --libs libchula`

cd CI
$CC -c -o lib-usage-1.o lib-usage-1.c ${HPACK_CFLAGS}
$CC -o lib-usage-1 lib-usage-1.o ${HPACK_LIBS}
./lib-usage-1

$CC -c -o lib-usage-2.o lib-usage-2.c ${CHULA_CFLAGS}
$CC -o lib-usage-2 lib-usage-2.o ${CHULA_LIBS}
./lib-usage-2
cd ..
