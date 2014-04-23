#!/bin/bash

# Play it safe: exit if something fails
set -e -o pipefail -o errtrace -o functrace

# Trigger a new tarball build
python tools/make-dist.py
