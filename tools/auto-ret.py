#!/usr/bin/env python

import re
import os
import string
import fnmatch

MACRO = """
#define %(func_name)s%(macro_args)s ({ \\
   ret_t __ret = %(func_orig)s %(macro_args)s; \\
   if (unlikely (__ret != ret_ok)) return __ret; \\
   __ret; \\
})
"""

def parse_file (fullpath_h, replacement_pair):
	with open(fullpath_h, 'r') as f:
		cont = f.read()

	# Find functions
	funcs = re.findall (r'ret_t\s+?[^\\#]+?\(.+?\);', cont, re.S)

	output = ''
	for func in funcs or []:
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
		func_args = [x.strip() for x in tmp[1].split(',')]
		func_argn = len(func_args)
		macro_args = '(%s)' %(','.join(string.uppercase[:func_argn]))

		output += MACRO %(locals())

	return output

def parse_dir (path_dir, path_header, replacement_pair):
	header = ''

	# Parse header files
	for root, dirnames, filenames in os.walk(path_dir):
		for filename in fnmatch.filter(filenames, "*.h"):
			fp = os.path.join (root, filename)
			header += parse_file(fp, replacement_pair)

	# Write target header
	with open(path_header, 'w+') as f:
		f.write(header)
	print '%s: DONE' %(path_header)


def main():
	parse_dir ("libchula", "libchula/chulada.h", ('chula_','chuli_'))
	parse_dir ("libhpack", "libhpack/hpack.h", ('hpack_','hp_'))


if __name__ == "__main__":
	main()
