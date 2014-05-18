#!/usr/bin/env python

import re
import os
import fnmatch
import argparse

MACRO = """
#define %(func_name)s%(func_suffix)s%(macro_args)s ({ \\
   ret_t __ret = %(func_orig)s %(macro_args)s; \\
   if (unlikely (__ret != ret_ok)) return __ret; \\
   __ret; \\
})
"""

SECTION = """
/* %(fullpath_h)s
 */
"""

def parse_file (fullpath_h, replacement_pair, func_suffix):
	with open(fullpath_h, 'r') as f:
		cont = f.read()

	# Find functions
	funcs = re.findall (r'ret_t\s+?[^\\#]+?\(.+?\);', cont, re.S)
	if not funcs:
		return ''

	output = SECTION%(locals())
	for func in funcs:
		# Remove new-lines and multiple spaces
		func = re.sub(' +',' ', func.replace('\n',' '))

		# Ignore function typedefs
		if re.findall('\(\s*?\*', func):
			continue

		# Parse it
		tmp = (re.findall (r'ret_t (.+?)\s*(\(.+\));', func) or [()])[0]
		if not tmp:
			print func
			raise SystemExit
		func_orig = tmp[0]
		func_name = func_orig.replace(replacement_pair[0], replacement_pair[1])

		# Arguments
		args = []
		func_args = [x.strip() for x in tmp[1].split(',')]
		for arg in func_args:
			if arg == '(void)':
				break
			args += [filter(lambda x: len(x), re.split(' |\*|\&|\)', arg))[-1]]
		macro_args = '(%s)' %(','.join(args))

		output += MACRO %(locals())

	return output

def parse_dir (path_dir, path_header, replacement_pair, func_suffix):
	header = ''
	# Parse header files
	for root, dirnames, filenames in os.walk(path_dir):
		for filename in fnmatch.filter(filenames, "*.h"):
			fp = os.path.abspath (os.path.join (root, filename))
			if fp == path_header:
				print "Ignoring %s" %(fp)
				continue
			header += parse_file(fp, replacement_pair, func_suffix)

	# Write target header
	with open(path_header, 'w+') as f:
		f.write(header)

	print '%s: %s definitions' %(path_header, header.count('#define'))


def main():
	# Command Line Args
	parser = argparse.ArgumentParser()
	parser.add_argument ('--path',        action="store", required=True, help="")
	parser.add_argument ('--output',      action="store", required=True, help="Output header file")
	parser.add_argument ('--replacement', action="store", default=":", help="Name replacement. Eg: chula_:chuli_")
	parser.add_argument ('--suffix',      action="store", default="",  help="Name suffix. Eg: _R")
	ns = parser.parse_args()
	if not ns:
		print ("ERROR: Couldn't parse parameters")
		raise SystemExit

	# Parse headers
	parse_dir (ns.path, os.path.abspath(ns.output), ns.replacement.split(':'), ns.suffix)

if __name__ == "__main__":
	main()
