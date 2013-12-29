#!/usr/bin/env python

import os, re, sys

CONFIG_H = """\
#ifndef CONFIG_H_
#define CONFIG_H_

/* Detected headers */
${{includes}}
/* Functions */
${{functions}}
/* Sizes */
${{sizes}}
#endif /* CONFIG_H_ */

#define PACKAGE_VERSION "@hpack_VERSION@"
"""

PATH_SRC = sys.argv[1]
PATH_BIN = sys.argv[2]

FILENAME_CMK = os.path.join (PATH_SRC, 'CMakeLists.txt')
FILENAME_NEW = os.path.join (PATH_BIN, 'config.h.in')

# Parse CMakeLists.txt
with open(FILENAME_CMK, 'r') as f:
    cont = f.read()

includes_t = ''
for h in re.findall (r'HTTP2D_CHECK_INCLUDE *\(.+? *(\w+)\)', cont, re.IGNORECASE):
	includes_t += '#cmakedefine %s\n' %(h)

functions_t = ''
for f in re.findall (r'CHECK_FUNCTION_EXISTS *\(.+? *(\w+)\)', cont, re.IGNORECASE):
	functions_t += '#cmakedefine %s\n' %(f)

sizes_t = ''
for h in re.findall (r'CHECK_TYPE_SIZE *\(.+? *(\w+)\)', cont, re.IGNORECASE):
    sizes_t += '@%s_CODE@\n' %(h)
    sizes_t += '#cmakedefine HAVE_%s\n' %(h)
    sizes_t += '#ifdef HAVE_%s\n' %(h)
    sizes_t += '# define HAVE_%s\n' %(h.replace('SIZEOF_',''))
    sizes_t += '#endif\n'

# Replacements
config_h = CONFIG_H
config_h = config_h.replace ("${{includes}}", includes_t)
config_h = config_h.replace ("${{functions}}", functions_t)
config_h = config_h.replace ("${{sizes}}", sizes_t)

# Write config.h
with open(FILENAME_NEW, 'w+') as f:
    f.write (config_h)
