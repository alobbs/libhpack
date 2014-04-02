#!/usr/bin/env python

import re
import sys
import subprocess
from threading import Thread

ESC   = chr(27) + '['
RESET = '%s0m' % (ESC)

def green (s):
    return ESC + '0;32m' + s + RESET
def red (s):
    return ESC + '0;31m' + s + RESET
def yellow (s):
    return ESC + '0;33m' + s + RESET
def red_ (s):
    return ESC + '4;31m' + s + RESET
def _red (s):
    return ESC + '0;41m' + s + RESET
def blue (s):
    return ESC + '0;34m' + s + RESET
def invert (s):
    return ESC + '7m' + s + RESET
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
			f_out.write ('\n' + invert(line.strip())+'\n')
			f_out.flush()
			continue

		val = re.findall (r'(\d+)\%: Checks: (\d+), Failures: (\d+), Errors: (\d+)', line)
		if not val:
			val = re.findall (r'.+ failed$', line)
			if val:
				line = line.replace ('failed', red_('failed'))

			val = re.findall (r' (Assertion .+ failed):', line)
			if val:
				line = line.replace (val[0], red_(val[0]))

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
		fail_c = (red,blue)[int(fail_d) <= 0](fail_d)
		per    = e[1][0]
		per_d  = int(decolor(per))
		per_c  = (red,green)[per_d == 100]("% 4d"%(per_d))

		print e[0], " "*(mlen-len(e[0])), '%s%%'%(per_c), \
			" "*(3-len(ok_d)), ok, \
			" "*(3-len(fail_d)), fail_c


def highlight_valgrind_run (f_in, f_out):
	def color_line (line,func):
		n = line.find("== ")
		if n == -1:
			return line
		return line[:n+3] + func(line[n+3:])

	outbuf = ''
	while True:
		line = f_in.readline()
		if not line:
			break

		if 'Invalid read' in line or \
		   'Process terminating with' in line:
			line = color_line(line.strip(), red) + '\n'
		elif 'definitely lost in' in line:
			line = color_line(line.strip(), yellow) + '\n'

		outbuf += line

	return outbuf


def run_ctest (binpath):
	def threaded_ctest(stdout):
		highlight_ctest_run (stdout, sys.stdout)

	def threaded_valgrind(stdout, outbuf):
		outbuf += [highlight_valgrind_run (stdout, sys.stderr)]

	stderr_out = []
	proc = subprocess.Popen (binpath, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
	thread1 = Thread (target=threaded_ctest,    args=((proc.stdout,)))
	thread2 = Thread (target=threaded_valgrind, args=((proc.stderr, stderr_out)))
	thread1.start()
	thread2.start()
	thread1.join()
	thread2.join()

	sys.stderr.write(stderr_out[0])
	sys.stderr.flush()

	proc.communicate()
	if proc.returncode != 0:
		print
		if proc.returncode < 0:
			print "[%s!!]"%(red_('CRASHED')),
		print red("Exited with error code %d\n"%(proc.returncode))

	return proc.returncode


def main():
	assert (len(sys.argv) >= 2)
	re = run_ctest (" ".join(sys.argv[1:]))
	sys.exit (re)


if __name__ == '__main__':
	main()
