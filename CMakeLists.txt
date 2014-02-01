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
    set (VALGRIND_OPTIONS --leak-check=full --num-callers=50)
endif()

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

# pkg-config
find_package(PkgConfig)

# Checks
include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)

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

macro(DEF_INCLUDE NAME VALUE)
  set (${NAME} ${VALUE})
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
HTTP2D_CHECK_INCLUDE (sys/time.h HAVE_SYS_TIME_H)
HTTP2D_CHECK_INCLUDE (sys/types.h HAVE_SYS_TYPES_H)
HTTP2D_CHECK_INCLUDE (sys/uio.h HAVE_SYS_UIO_H)
HTTP2D_CHECK_INCLUDE (sys/un.h HAVE_SYS_UN_H)
HTTP2D_CHECK_INCLUDE (sys/utsname.h HAVE_SYS_UTSNAME_H)
HTTP2D_CHECK_INCLUDE (sys/varargs.h HAVE_SYS_VARARGS_H)
HTTP2D_CHECK_INCLUDE (sys/wait.h HAVE_SYS_WAIT_H)
HTTP2D_CHECK_INCLUDE (syslog.h HAVE_SYSLOG_H)
HTTP2D_CHECK_INCLUDE (time.h HAVE_TIME_H)
HTTP2D_CHECK_INCLUDE (paths.h HAVE_PATHS_H)

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
CHECK_FUNCTION_EXISTS (readdir HAVE_READDIR)
CHECK_FUNCTION_EXISTS (readdir_r HAVE_READDIR_R)
CHECK_FUNCTION_EXISTS (getrlimit HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS (getdtablesize HAVE_GETDTABLESIZE)
CHECK_FUNCTION_EXISTS (getpwnam_r HAVE_GETPWNAM_R)
CHECK_FUNCTION_EXISTS (getpwuid_r HAVE_GETPWUID_R)
CHECK_FUNCTION_EXISTS (getgrnam_r HAVE_GETGRNAM_R)
CHECK_FUNCTION_EXISTS (getgrgid_r HAVE_GETGRGID_R)
CHECK_FUNCTION_EXISTS (gmtime_r HAVE_GMTIME_R)
CHECK_FUNCTION_EXISTS (localtime_r HAVE_LOCALTIME_R)
CHECK_FUNCTION_EXISTS (gethostname HAVE_GETHOSTNAME)
CHECK_FUNCTION_EXISTS (inet_ntoa HAVE_INET_NTOP)
CHECK_FUNCTION_EXISTS (srandomdev HAVE_SRANDOMDEV)
CHECK_FUNCTION_EXISTS (srandom HAVE_SRANDOM)
CHECK_FUNCTION_EXISTS (random HAVE_RANDOM)
CHECK_FUNCTION_EXISTS (backtrace HAVE_BACKTRACE)
SET(CMAKE_EXTRA_INCLUDE_FILES)

# Platform
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    DEF_INCLUDE (LINUX TRUE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    DEF_INCLUDE (OSX TRUE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "(FreeBSD|OpenBSD|NetBSD)")
    DEF_INCLUDE (BSD TRUE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "(Solaris|SunOS)")
    DEF_INCLUDE (SOLARIS TRUE)
endif()

DEF_INCLUDE (PACKAGE_VERSION "${hpack_VERSION}")

# Code tests
CHECK_C_SOURCE_COMPILES("
#include <sys/types.h>
#include <dirent.h>
int main() {
    struct dirent dir, *dirp;
    DIR *mydir;
    dirp = readdir_r(mydir, &dir);
}
" HAVE_READDIR_R_2)

CHECK_C_SOURCE_COMPILES("
#include <sys/types.h>
#include <dirent.h>
int main() {
    struct dirent dir, *dirp;
    DIR *mydir;
    int rc;
    rc = readdir_r(mydir, &dir, &dirp);
}
" HAVE_READDIR_R_3)

CHECK_C_SOURCE_COMPILES("
#include <pwd.h>
#include <stdlib.h>
int main() {
    getpwnam_r(NULL,NULL,NULL,0,NULL);
}
" HAVE_GETPWNAM_R_5)

CHECK_C_SOURCE_COMPILES("
#include <pwd.h>
#include <stdlib.h>
int main() {
    getpwnam_r(NULL,NULL,NULL,0);
}
" HAVE_GETPWNAM_R_4)

CHECK_C_SOURCE_COMPILES("
#include <pwd.h>
#include <stdlib.h>
int main() {
    getpwuid_r(0,NULL,NULL,0,NULL);
}
" HAVE_GETPWUID_R_5)

CHECK_C_SOURCE_COMPILES("
#include <pwd.h>
#include <stdlib.h>
int main() {
    getpwuid_r(0,NULL,NULL,0);
}
" HAVE_GETPWUID_R_4)

CHECK_C_SOURCE_COMPILES("
#include <grp.h>
#include <stdlib.h>
int main() {
    getgrnam_r(NULL,NULL,NULL,0,NULL);
}
" HAVE_GETGRNAM_R_5)

CHECK_C_SOURCE_COMPILES("
#include <grp.h>
#include <stdlib.h>
int main() {
    getgrnam_r(NULL,NULL,NULL,0);
}
" HAVE_GETGRNAM_R_4)

CHECK_C_SOURCE_COMPILES("
#include <grp.h>
#include <stdlib.h>
int main() {
    getgrgid_r(0,NULL,NULL,0,NULL);
}
" HAVE_GETGRGID_R_5)

CHECK_C_SOURCE_COMPILES("
#include <grp.h>
#include <stdlib.h>
int main() {
    getgrgid_r(0,NULL,NULL,0);
}
" HAVE_GETGRGID_R_4)

CHECK_C_SOURCE_COMPILES("
#include <time.h>
int main() {
    struct tm stm;
    stm.tm_gmtoff = 0;
}
" HAVE_STRUCT_TM_GMTOFF)

CHECK_C_SOURCE_COMPILES("
#include <time.h>
int main() {
    int res;
    res = timezone / 60;
}
" HAVE_INT_TIMEZONE)

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

  if(VALGRIND)
    add_test("valgrind-hpack" ${VALGRIND} ${VALGRIND_OPTIONS} test/test_libhpack)
    add_test("valgrind-chula" ${VALGRIND} ${VALGRIND_OPTIONS} libchula/test/chula_test)
  else()
    add_test(hpack test/test_libhpack)
    add_test(chula libchula/test/chula_test)
  endif()
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
