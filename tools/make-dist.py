import os
import re
import shutil
import tempfile
import subprocess

def run (cmd, fatal=True):
	re = os.system(cmd)
	if fatal:
		assert (re == 0)

# Temporary directory
tmp_git = tempfile.mkdtemp()
tmp_bin = tempfile.mkdtemp()

# Checkout
os.chdir (tmp_git);
run ("git clone --recursive https://github.com/alobbs/libhpack.git")

# Configure
os.makedirs ("libhpack/build")
os.chdir ("libhpack/build")
run ("cmake ..")

# Copy html doc to source tree
run ("make doc")
run ("cp -rv doc/html ../doc")

# Dist
for line in os.popen ("make package_source", 'r'):
	print line,
	tmp = re.findall (r' (/.*/libhpack-.+\.tar\.bz2)', line)
	if tmp:
		src_tgz = tmp[0]

# Build test
os.chdir (tmp_bin)
run ("tar xfvj %s"%(src_tgz))

src_dir = os.path.basename(src_tgz).replace('.tar.bz2','')
os.chdir (src_dir)
run ("make debug")
run ("make test")

shutil.rmtree (tmp_bin)
print ("\n%s is ready" %(src_tgz))
