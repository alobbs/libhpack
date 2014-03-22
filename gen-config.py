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
/* Definitions */
${{definitions}}

#endif /* CONFIG_H_ */
"""

PATH_SRC = sys.argv[1]
PATH_BIN = sys.argv[2]

FILENAME_CMK = os.path.join (PATH_SRC, 'CMakeLists.txt')
FILENAME_NEW = os.path.join (PATH_BIN, 'config.h.in')

# Parse CMakeLists.txt
with open(FILENAME_CMK, 'r') as f:
    cont = f.read()

includes_t = ''
for h in re.findall (r'HPACK_CHECK_INCLUDE *\(.+? *(\w+)\)', cont, re.IGNORECASE):
	includes_t += '#cmakedefine %s\n' %(h)

functions_t = ''
for f in re.findall (r'CHECK_FUNCTION_EXISTS *\(.+? *(\w+)\)', cont, re.IGNORECASE):
	functions_t += '#cmakedefine %s\n' %(f)

for f in re.findall (r'CHECK_C_SOURCE_COMPILES *\(.+?(HAVE_.+?)\)\n', cont, re.S):
	functions_t += '#cmakedefine %s\n' %(f)
for f in re.findall (r'CHECK_C_SOURCE_RUNS *\(.+?(HAVE_.+?)\)\n', cont, re.S):
	functions_t += '#cmakedefine %s\n' %(f)

definitions_t = ''
for f in re.findall (r'DEF_SET *\((\w+)? +(.+?)\)', cont, re.IGNORECASE):
	definitions_t += '#cmakedefine %s %s\n' %(f[0], f[1])
for f in re.findall (r'DEF_SET_IFNDEF *\((\w+)? +(.+?)\)', cont, re.IGNORECASE):
	definitions_t += '#ifndef %s\n' %(f[0])
	definitions_t += '#cmakedefine %s %s\n' %(f[0], f[1])
	definitions_t += '#endif\n'
for f in re.findall (r'DEF_DEFINE *\((\w+)?\)', cont, re.IGNORECASE):
	definitions_t += '#cmakedefine %s\n' %(f)

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
config_h = config_h.replace ("${{definitions}}", definitions_t)

# Write config.h
with open(FILENAME_NEW, 'w+') as f:
    f.write (config_h)
