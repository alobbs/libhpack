#!/usr/bin/env python

import re
import sys

ESC   = chr(27) + '['
RESET = '%s0m' % (ESC)

def green (s):
    return ESC + '0;32m' + s + RESET
def red (s):
    return ESC + '0;31m' + s + RESET
def blue (s):
    return ESC + '0;34m' + s + RESET
def decolor (s):
	return s[7:-4]

def highlight_ctest_run (f_in, f_out):
	summary = []

	while True:
		line = f_in.readline()
		if not line:
			break

		val = re.findall (r'Running suite\(s\): (.+)', line)
		if val:
			last_title = val[0]

		val = re.findall (r'(\d+)\%: Checks: (\d+), Failures: (\d+), Errors: (\d+)', line)
		if not val:
			f_out.write (line)
			f_out.flush()
			continue

		# Highlight
		val = list(val[0])

		val[0] = (red,green)[int(val[0]) == 100](val[0])
		val[1] = blue(val[1])
		val[2] = blue(val[2])
		val[3] = (green,red)[int(val[3]) > 0](val[3])

		new_line = "%s%%: Checks: %s, Failures: %s, Errors: %s\n" %(val[0], val[1], val[2], val[3])
		f_out.write (new_line)
		f_out.flush()

		summary += [[last_title, val]]

	# Global report
	mlen = max([len(e[0]) for e in summary])

	print
	for e in summary:
		ok     = e[1][1]
		ok_d   = decolor(ok)
		fail   = e[1][2]
		fail_d = decolor(fail)

		print e[0], " "*(mlen-len(e[0])), '%s%%'%(e[1][0]), \
			" "*(3-len(ok_d)), ok, \
			" "*(3-len(fail_d)), fail

def main():
	highlight_ctest_run (sys.stdin, sys.stdout)

if __name__ == '__main__':
	main()
