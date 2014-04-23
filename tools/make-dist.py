#!/usr/bin/env python

import os
import re
import shutil
import argparse
import tempfile
import subprocess

# Parse command line parameters
parser = argparse.ArgumentParser()
parser.add_argument ('--no-tests', action="store_true", default=False, help="Skip the QA test bench execution")
ns = parser.parse_args()
if not ns:
    print ("ERROR: Couldn't parse parameters")
    raise SystemExit

def run (cmd, fatal=True):
	print "+ %s"%(cmd)
	re = os.system(cmd)
	if fatal:
		assert re==0, cmd

# Temporary directory
tmp_git = tempfile.mkdtemp()
tmp_bin = tempfile.mkdtemp()

print ("Temp Git: %s"%(tmp_git))
print ("Temp Bin: %s"%(tmp_bin))

# Checkout
os.chdir (tmp_git);
run ("git clone --recursive https://github.com/alobbs/libhpack.git")

# Generate the Huffman table file
os.chdir ("libhpack")
run ("./libhpack/huffman-gen.py -v")

# Configure
os.makedirs ("build")
os.chdir ("build")
run ("cmake -DUSE_VALGRIND=NO ..")

# Copy html doc to source tree
run ("make doc")
run ("cp -rv doc/html ../doc")

# Build tarball
with os.popen ("make package_source", 'r') as f:
	for line in f:
		print line,
		tmp = re.findall (r' (/.*/libhpack-.+\.tar\.bz2)', line)
		if tmp:
			src_tgz = tmp[0]

# Test: Unpack
os.chdir (tmp_bin)
run ("tar xfvj %s"%(src_tgz))

# Test: Build
src_dir = os.path.basename(src_tgz).replace('.tar.bz2','')
os.chdir (src_dir)
run ("make BUILD_DOCS=OFF debug")

# Test: QA
if not ns.no_tests:
	run ("make test")

shutil.rmtree (tmp_bin)
print ("\n%s is ready" %(src_tgz))
