#!/bin/bash

# Play it safe: exit if something fails
set -x -e -o pipefail -o errtrace -o functrace

# Where is it running?
lsb_release -a
uname -a

# Before Install
sudo apt-get update -qq

# Installation
sudo add-apt-repository -y "deb http://archive.ubuntu.com/ubuntu/ precise main universe"
sudo apt-get install -y cmake doxygen python-sphinx gcc make check valgrind
sudo pip install cpp-coveralls
