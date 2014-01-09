# CMake version check
cmake_minimum_required(VERSION 2.8)

# Project
set(PROJECT_NAME_STR libhpack)
project(${PROJECT_NAME_STR} C CXX)

# 1. If the library source code has changed at all since the last update, then increment revision.
# 2. If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.
# 3. If any interfaces have been added since the last public release, then increment age.
# 4. If any interfaces have been removed since the last public release, then set age to 0.
set(hpack_SOVERSION_CURRENT  1)
set(hpack_SOVERSION_REVISION 0)
set(hpack_SOVERSION_AGE      0)

math(EXPR hpack_SOVERSION_MAJOR "${hpack_SOVERSION_CURRENT} - ${hpack_SOVERSION_AGE}")
math(EXPR hpack_SOVERSION_MINOR "${hpack_SOVERSION_AGE}")
math(EXPR hpack_SOVERSION_PATCH "${hpack_SOVERSION_REVISION}")

set(hpack_VERSION ${hpack_SOVERSION_MAJOR}.${hpack_SOVERSION_MINOR}.${hpack_SOVERSION_PATCH})
set(hpack_SOVERSION ${hpack_SOVERSION_MAJOR})

# Options
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_DOCS  "Build the documentation" ON)

option(ENABLE_GCOV "Coverage support" false)
if(ENABLE_GCOV)
  set(CMAKE_C_FLAGS "-coverage ${CMAKE_C_FLAGS}")
endif()

option (USE_VALGRIND "Use valgrind when testing" true)
if(USE_VALGRIND)
  find_program(VALGRIND valgrind)
endif()

# Checks
include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckFunctionExists)

# Includes
include_directories(${CMAKE_SOURCE_DIR} ${PROJECT_BINARY_DIR})

# Cmake macros
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Headers
macro(HTTP2D_CHECK_INCLUDE INCLUDE VARIABLE)
  CHECK_INCLUDE_FILES (${INCLUDE} ${VARIABLE})
  if(${VARIABLE})
    set(HTTP2D_ALL_INCLUDES "${INCLUDE};${HTTP2D_ALL_INCLUDES}")
  endif()
endmacro()

HTTP2D_CHECK_INCLUDE (arpa/inet.h HAVE_ARPA_INET_H)
HTTP2D_CHECK_INCLUDE (endian.h HAVE_ENDIAN_H)
HTTP2D_CHECK_INCLUDE (execinfo.h HAVE_EXECINFO_H)
HTTP2D_CHECK_INCLUDE (fcntl.h HAVE_FCNTL_H)
HTTP2D_CHECK_INCLUDE (grp.h HAVE_GRP_H)
HTTP2D_CHECK_INCLUDE (inttypes.h HAVE_INTTYPES_H)
HTTP2D_CHECK_INCLUDE (machine/endian.h HAVE_MACHINE_ENDIAN_H)
HTTP2D_CHECK_INCLUDE (netdb.h HAVE_NETDB_H)
HTTP2D_CHECK_INCLUDE (netinet/in.h HAVE_NETINET_IN_H)
HTTP2D_CHECK_INCLUDE (netinet/tcp.h HAVE_NETINET_TCP_H)
HTTP2D_CHECK_INCLUDE (pwd.h HAVE_PWD_H)
HTTP2D_CHECK_INCLUDE (resource.h HAVE_RESOURCE_H)
HTTP2D_CHECK_INCLUDE (stdint.h HAVE_STDINT_H)
HTTP2D_CHECK_INCLUDE (sys/endian.h HAVE_SYS_ENDIAN_H)
HTTP2D_CHECK_INCLUDE (sys/fcntl.h HAVE_SYS_FCNTL_H)
HTTP2D_CHECK_INCLUDE (sys/filio.h HAVE_SYS_FILIO_H)
HTTP2D_CHECK_INCLUDE (sys/ioctl.h HAVE_SYS_FILIO_H)
HTTP2D_CHECK_INCLUDE (sys/isa_defs.h HAVE_SYS_ISA_DEFS_H)
HTTP2D_CHECK_INCLUDE (sys/machine.h HAVE_SYS_MACHINE_H)
HTTP2D_CHECK_INCLUDE (sys/resource.h HAVE_SYS_RESOURCE_H)
HTTP2D_CHECK_INCLUDE (sys/socket.h HAVE_SYS_SOCKET_H)
HTTP2D_CHECK_INCLUDE (sys/socket.h HAVE_SYS_SOCKET_H)
HTTP2D_CHECK_INCLUDE (sys/time.h HAVE_SYS_TIME_H)
HTTP2D_CHECK_INCLUDE (sys/types.h HAVE_SYS_TYPES_H)
HTTP2D_CHECK_INCLUDE (sys/uio.h HAVE_SYS_UIO_H)
HTTP2D_CHECK_INCLUDE (sys/un.h HAVE_SYS_UN_H)
HTTP2D_CHECK_INCLUDE (sys/utsname.h HAVE_SYS_UTSNAME_H)
HTTP2D_CHECK_INCLUDE (sys/varargs.h HAVE_SYS_VARARGS_H)
HTTP2D_CHECK_INCLUDE (sys/wait.h HAVE_SYS_WAIT_H)
HTTP2D_CHECK_INCLUDE (syslog.h HAVE_SYSLOG_H)
HTTP2D_CHECK_INCLUDE (time.h HAVE_TIME_H)

# Structs
SET(CMAKE_EXTRA_INCLUDE_FILES ${HTTP2D_ALL_INCLUDES})
CHECK_TYPE_SIZE ("int" SIZEOF_INT)
CHECK_TYPE_SIZE ("long" SIZEOF_LONG)
CHECK_TYPE_SIZE ("unsigned int" SIZEOF_UNSIGNED_INT)
CHECK_TYPE_SIZE ("unsigned long" SIZEOF_UNSIGNED_LONG)
CHECK_TYPE_SIZE ("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE ("time_t" SIZEOF_TIME_T)
CHECK_TYPE_SIZE ("struct in6_addr" SIZEOF_IN6_ADDR)
CHECK_TYPE_SIZE ("struct sockaddr_un" SIZEOF_SOCKADDR_UN)
CHECK_TYPE_SIZE ("struct sockaddr_in6" SIZEOF_SOCKADDR_IN6)
SET(CMAKE_EXTRA_INCLUDE_FILES)

# Functions
SET(CMAKE_EXTRA_INCLUDE_FILES ${HTTP2D_ALL_INCLUDES})
CHECK_FUNCTION_EXISTS (strnstr HAVE_STRNSTR)
CHECK_FUNCTION_EXISTS (strcasestr HAVE_STRCASESTR)
CHECK_FUNCTION_EXISTS (strlcat HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS (rpl_malloc HAVE_MALLOC)
SET(CMAKE_EXTRA_INCLUDE_FILES)

# Libraries
find_library(LIBM NAMES m)

if (CMAKE_COMPILER_IS_GNUCC)
  add_definitions (-std=gnu99)
endif (CMAKE_COMPILER_IS_GNUCC)

# Library source code
add_subdirectory(libchula)
add_subdirectory(libhpack)

if(BUILD_TESTS)
  include(CTest)
  enable_testing(true)
  add_subdirectory(test)
  add_test(hpack test/test_libhpack)
  add_test(chula libchula/test/chula_test)
  add_test(doxy-coverage doc/external/doxy-coverage/doxy-coverage.py)
endif(BUILD_TESTS)

if(BUILD_DOCS)
  add_subdirectory(doc)
  if(BUILD_TESTS)
    add_test(doxy-coverage ${CMAKE_CURRENT_SOURCE_DIR}/doc/external/doxy-coverage/doxy-coverage.py ${CMAKE_CURRENT_BINARY_DIR}/doc/xml --noerror)
  endif(BUILD_TESTS)
endif(BUILD_DOCS)

# config.h
execute_process (COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen-config.py ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE (${CMAKE_CURRENT_BINARY_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h IMMEDIATE)

# Final license reminder
MESSAGE ("
+------------------------------------------------------------------+
| License:                                                         |
| This software is subject to the BSD License, available in this   |
| distribution in the file LICENSE. Please, remember that any copy |
| distribution or modification of the software is subject to it.   |
+------------------------------------------------------------------+

Thank you for using ${PROJECT_NAME_STR}.
")
