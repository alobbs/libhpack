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

# Paths
from_here = lambda x: os.path.normpath (os.path.dirname (os.path.realpath(__file__)) + x)

PATH_CHULA      = from_here('/../libchula')
PATH_CHULA_QA   = from_here('/../libchula-qa')
PATH_HPACK      = from_here('/../libhpack')
PATH_CHULA_TEST = from_here('/../libchula/test')
PATH_HPACK_TEST = from_here('/../test')
PATH_HPACK_CI   = from_here('')


def _find_files (paths, match_filters, match_postskip = None):
	h_files = []
	for d in paths:
		for root, dirnames, filenames in os.walk(d):
			for m_fil in match_filters:
				for filename in fnmatch.filter(filenames, m_fil):
					fp = os.path.join(root, filename)
					if match_postskip and match_postskip in fp:
						continue
					h_files.append (fp)
	return h_files

def check_ifdef_HAVE():
	"""Find conditional inclusions in header files.

	Ideally, none of the header files shipped in the library should
	use "#ifdef HAVE_*" preprocessor entries, so the project's
	config.h file doesn't have to by included along with them. This
	function finds the .h files that should be worked out to avoid
	these conditional inclusions.
	"""

	def _check_header (h_path):
		with open(h_path,'r') as f:
			haves = re.findall(r'\#ifdef\s+(HAVE_.+)\s', f.read())
			if haves:
				return ["%s - Conditional includes %s" %(h_path, ', '.join(haves))]
			return []

	errors = []
	for h_path in _find_files ([PATH_CHULA, PATH_HPACK], ['*.h'], '-internal'):
		errors += _check_header(h_path)

	return errors

def check_common_internal():
	"""Makes sure .c files include the common-internal.h file.

	This function checks the .c files to make sure they include the
	common-internal.h header file. It isn't strictly necessary,
	however it is a good practice that we'd like to enforce. In the
	past we've found a few bugs caused by the lack of some of the
	headers that are pulled from common-internal.h.
	"""

	def _check_header (c_path):
		with open(c_path,'r') as f:
			if not 'common-internal.h' in f.read():
				return ["%s - missing inclusion: common-internal.h" %(c_path)]
			return []

	errors = []
	for c_path in _find_files ([PATH_CHULA], ['*.c'], '/test/'):
		errors += _check_header(c_path)

	return errors

def check_local_includes():
	"""Makes sure .h files don't use local inclusions (with "")

	Headers files will be exported, and therefore they should not make
	use of local includes. Instead, they ought to use the public
	header path.
	"""

	def _check_header (h_path):
		with open(h_path,'r') as f:
			local_includes = re.findall(r'\#\s*include\s+"(.+)"', f.read())
			if local_includes:
				return ["%s - has local includes %s" %(h_path, ', '.join(local_includes))]
			return []

	errors = []
	for h_path in _find_files ([PATH_CHULA, PATH_HPACK], ['*.h']):
		errors += _check_header(h_path)

	return errors

def check_cstrings_funcs():
	"""Cheks if chula's func wrapper for missing libc functions are used

	This function checks the .c files to make sure they use the
	cstrings wrappers of the string handling functions that may be
	missing from some of the systems where libhack should work.
	"""

	FUNCS = ('strsep', 'strnstr', 'strcasestr', 'strlcat')

	def _check_header (c_path):
		with open(c_path,'r') as f:
			cont = f.read()
 			funcs_found = []
			for func in FUNCS:
				found = re.findall ('[^_](%s)[\s\n]*\('%(func), cont)
				if found:
					funcs_found.append(func)

			if funcs_found:
				return ['%s - Unwrapped functions: %s'%(c_path, ', '.join(funcs_found))]
			return []

	errors = []
	for c_path in _find_files ([PATH_CHULA, PATH_HPACK], ['*.c'], 'libchula/cstrings.c'):
		errors += _check_header(c_path)

	return errors


def code_vs_tests():
	"""Measures the amount of code and its testing counterpart
	"""

	code_size = 0
	test_size = 0

	for c_path in _find_files ([PATH_CHULA, PATH_HPACK], ['*.c','*.h','*.py']):
		code_size += os.path.getsize (c_path)
	for c_path in _find_files ([PATH_CHULA_TEST, PATH_HPACK_TEST, PATH_HPACK_CI, PATH_CHULA_QA], ['*.c','*.h','*.py']):
		test_size += os.path.getsize (c_path)

	percent = (test_size*100)/float(code_size)
	if percent < 50:
		print "[WARNING]:",
	print "Testing/Code ratio: %0.2f%% (code=%dKb test=%dKb)" %(percent, code_size/1024, test_size/1024)
	return []


def main():
	errors = []
	errors += check_ifdef_HAVE()
	errors += check_common_internal()
	errors += check_local_includes()
	errors += check_cstrings_funcs()
	errors += code_vs_tests()

	if len(errors):
		print "Total %d errors" %(len(errors))
		print "\n".join([' %s'%(e) for e in errors])
	return len(errors)

if __name__ == "__main__":
	sys.exit(main())
