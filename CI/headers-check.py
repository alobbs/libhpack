#!/usr/bin/env python

# All files in libhpack are Copyright (C) 2014 Alvaro Lopez Ortega.
#
#   Authors:
#    # Alvaro Lopez Ortega <alvaro@gnu.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    # Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#    # Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import re
import sys
import fnmatch

def _find_h_files (paths):
	h_files = []
	for d in paths:
		for root, dirnames, filenames in os.walk(d):
			for filename in fnmatch.filter(filenames, '*.h'):
				if not '-internal' in filename:
					h_files.append(os.path.join(root, filename))
	return h_files

def _check_header (h_path):
	with open(h_path,'r') as f:
		return re.findall(r'\#ifdef\s+(HAVE_.+)\s', f.read()) or []

def _print_report (offenders):
	if not offenders:
		print "Everything okay!"
		return

	for h in offenders:
		print "[% 2d errors] %s:\n\t%s" %(len(offenders[h]), h, ', '.join (offenders[h]))

def main():
	offenders = {}
	for h_path in _find_h_files (sys.argv[1:]):
		errors = _check_header(h_path)
		if errors:
			offenders[h_path] = errors

	_print_report (offenders)
	return len(offenders)

if __name__ == "__main__":
	sys.exit(main())
