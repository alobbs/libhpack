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

# Standard replacements
set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(includedir  ${CMAKE_INSTALL_PREFIX}/include)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

# Options
option(BUILD_DOCS "Build the documentation" ON)

option(ENABLE_GCOV "Coverage support" OFF)
if(ENABLE_GCOV)
  set(CMAKE_C_FLAGS "-coverage ${CMAKE_C_FLAGS}")
endif()

option (USE_VALGRIND "Use valgrind when testing" OFF)
if(USE_VALGRIND)
    find_program(VALGRIND valgrind)
    set (VALGRIND_OPTIONS --leak-check=full --num-callers=50)
endif()

# Includes
include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(TestBigEndian)
include(CMakePushCheckState)

# Compiler options
add_definitions (-std=gnu99)
add_definitions (-D_GNU_SOURCE)
set (CMAKE_C_FLAGS_DEBUG "-O0 -Wall -Wextra")

if (CMAKE_COMPILER_IS_GNUCC)
  MESSAGE (STATUS "Compiler is GCC")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3 -gdwarf-2")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  MESSAGE (STATUS "Compiler is Clang")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
else()
  MESSAGE (STATUS "Unknown compiler")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
endif (CMAKE_COMPILER_IS_GNUCC)

# pkg-config
find_package(PkgConfig)

# Includes
include_directories(${CMAKE_SOURCE_DIR} ${PROJECT_BINARY_DIR})

# Cmake macros
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Headers
macro(HPACK_CHECK_INCLUDE INCLUDE VARIABLE)
  CHECK_INCLUDE_FILES (${INCLUDE} ${VARIABLE})
  if(${VARIABLE})
    set(HPACK_ALL_INCLUDES "${INCLUDE};${HPACK_ALL_INCLUDES}")
  endif()
endmacro()

macro(DEF_SET_IFNDEF NAME VALUE)
  set (${NAME} ${VALUE})
endmacro()

macro(DEF_SET NAME VALUE)
  set (${NAME} ${VALUE})
endmacro()

macro(DEF_DEFINE NAME)
  set (${NAME})
endmacro()

HPACK_CHECK_INCLUDE (arpa/inet.h HAVE_ARPA_INET_H)
HPACK_CHECK_INCLUDE (endian.h HAVE_ENDIAN_H)
HPACK_CHECK_INCLUDE (execinfo.h HAVE_EXECINFO_H)
HPACK_CHECK_INCLUDE (fcntl.h HAVE_FCNTL_H)
HPACK_CHECK_INCLUDE (grp.h HAVE_GRP_H)
HPACK_CHECK_INCLUDE (inttypes.h HAVE_INTTYPES_H)
HPACK_CHECK_INCLUDE (machine/endian.h HAVE_MACHINE_ENDIAN_H)
HPACK_CHECK_INCLUDE (netdb.h HAVE_NETDB_H)
HPACK_CHECK_INCLUDE (netinet/in.h HAVE_NETINET_IN_H)
HPACK_CHECK_INCLUDE (netinet/tcp.h HAVE_NETINET_TCP_H)
HPACK_CHECK_INCLUDE (pwd.h HAVE_PWD_H)
HPACK_CHECK_INCLUDE (resource.h HAVE_RESOURCE_H)
HPACK_CHECK_INCLUDE (stdint.h HAVE_STDINT_H)
HPACK_CHECK_INCLUDE (sys/endian.h HAVE_SYS_ENDIAN_H)
HPACK_CHECK_INCLUDE (sys/fcntl.h HAVE_SYS_FCNTL_H)
HPACK_CHECK_INCLUDE (sys/filio.h HAVE_SYS_FILIO_H)
HPACK_CHECK_INCLUDE (sys/ioctl.h HAVE_SYS_IOCTL_H)
HPACK_CHECK_INCLUDE (sys/isa_defs.h HAVE_SYS_ISA_DEFS_H)
HPACK_CHECK_INCLUDE (sys/machine.h HAVE_SYS_MACHINE_H)
HPACK_CHECK_INCLUDE (sys/resource.h HAVE_SYS_RESOURCE_H)
HPACK_CHECK_INCLUDE (sys/socket.h HAVE_SYS_SOCKET_H)
HPACK_CHECK_INCLUDE (sys/time.h HAVE_SYS_TIME_H)
HPACK_CHECK_INCLUDE (sys/types.h HAVE_SYS_TYPES_H)
HPACK_CHECK_INCLUDE (sys/uio.h HAVE_SYS_UIO_H)
HPACK_CHECK_INCLUDE (sys/un.h HAVE_SYS_UN_H)
HPACK_CHECK_INCLUDE (sys/utsname.h HAVE_SYS_UTSNAME_H)
HPACK_CHECK_INCLUDE (sys/varargs.h HAVE_SYS_VARARGS_H)
HPACK_CHECK_INCLUDE (sys/wait.h HAVE_SYS_WAIT_H)
HPACK_CHECK_INCLUDE (sys/mman.h HAVE_SYS_MMAN_H)
HPACK_CHECK_INCLUDE (syslog.h HAVE_SYSLOG_H)
HPACK_CHECK_INCLUDE (time.h HAVE_TIME_H)
HPACK_CHECK_INCLUDE (paths.h HAVE_PATHS_H)
HPACK_CHECK_INCLUDE (string.h HAVE_STRING_H)
HPACK_CHECK_INCLUDE (strings.h HAVE_STRINGS_H)
HPACK_CHECK_INCLUDE (malloc.h HAVE_MALLOC_H)
HPACK_CHECK_INCLUDE (malloc/malloc.h HAVE_MALLOC_MALLOC_H)

# Structs
SET(CMAKE_EXTRA_INCLUDE_FILES ${HPACK_ALL_INCLUDES})
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
SET(CMAKE_EXTRA_INCLUDE_FILES ${HPACK_ALL_INCLUDES})
CHECK_FUNCTION_EXISTS (malloc HAVE_MALLOC)
CHECK_FUNCTION_EXISTS (realloc HAVE_REALLOC)
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
CHECK_FUNCTION_EXISTS (strsep HAVE_STRSEP)
CHECK_FUNCTION_EXISTS (strnstr HAVE_STRNSTR)
CHECK_FUNCTION_EXISTS (strcasestr HAVE_STRCASESTR)
CHECK_FUNCTION_EXISTS (strlcat HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS (strerror_r HAVE_STRERROR_R)
CHECK_FUNCTION_EXISTS (malloc_usable_size HAVE_MALLOC_USABLE_SIZE)
CHECK_FUNCTION_EXISTS (malloc_size HAVE_MALLOC_SIZE)
CHECK_FUNCTION_EXISTS (malloc_default_zone HAVE_MALLOC_DEFAULT_ZONE)
CHECK_FUNCTION_EXISTS (__malloc_hook HAVE_MALLOC_HOOK)
SET(CMAKE_EXTRA_INCLUDE_FILES)

# Platform
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    DEF_SET (LINUX TRUE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    DEF_SET (OSX TRUE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "(FreeBSD|OpenBSD|NetBSD)")
    DEF_SET (BSD TRUE)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "(Solaris|SunOS)")
    DEF_SET (SOLARIS TRUE)
endif()

DEF_SET (PACKAGE_VERSION "${hpack_VERSION}")

# System endianness
test_big_endian(WORDS_BIGENDIAN)

if(WORDS_BIGENDIAN)
  set(byteorder 4321)
else(WORDS_BIGENDIAN)
  set(byteorder 1234)
endif(WORDS_BIGENDIAN)

MESSAGE (STATUS "Byte order ${byteorder}")
DEF_SET_IFNDEF (BYTE_ORDER ${byteorder})
DEF_DEFINE (WORDS_BIGENDIAN)

INCLUDE(CheckFileOffsetBits)
CHECK__FILE_OFFSET_BITS()
IF(_FILE_OFFSET_BITS)
    add_definitions (-D_FILE_OFFSET_BITS=${_FILE_OFFSET_BITS})
ENDIF(_FILE_OFFSET_BITS)

# Code tests
cmake_push_check_state()

get_directory_property(CMAKE_REQUIRED_FLAGS COMPILE_FLAGS)
get_directory_property(compile_definitions COMPILE_DEFINITIONS)
foreach (def ${compile_definitions})
  list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${def}")
endforeach (def ${compile_definitions})

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

CHECK_C_SOURCE_RUNS("
#include <string.h>
#include <errno.h>
int main () {
  char buffer[1024];
  char *string = strerror_r (EACCES, buffer, sizeof(buffer));
  return (!string || !string[0]) ? 99 : 0;
}" HAVE_GLIBC_STRERROR_R)

CHECK_C_SOURCE_RUNS("
#include <string.h>
#include <errno.h>
int main () {
  char buffer[1024];
  int error = strerror_r (EACCES, buffer, sizeof(buffer));
  return (!buffer[0] || error) ? 99: 0;
}" HAVE_POSIX_STRERROR_R)

cmake_pop_check_state()

# Libraries
find_library(LIBM NAMES m)

# Library source code
if(BUILD_DOCS)
  add_custom_target (doc)
  add_subdirectory(doc)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/external/doxy-coverage)
    add_test(doxy-coverage ${CMAKE_CURRENT_SOURCE_DIR}/doc/external/doxy-coverage/doxy-coverage.py ${CMAKE_CURRENT_BINARY_DIR}/doc/xml --noerror)
  endif()
else()
  add_custom_target (doc)
endif(BUILD_DOCS)

add_subdirectory(libchula)
add_subdirectory(libchula-qa)
add_subdirectory(libhpack)

# Testing
include(CTest)
enable_testing(true)
add_subdirectory(test)

if(USE_VALGRIND)
  message(STATUS "Testing run under valgrind")
  add_test("valgrind-hpack" ${VALGRIND} ${VALGRIND_OPTIONS} test/test_libhpack)
  add_test("valgrind-chula" ${VALGRIND} ${VALGRIND_OPTIONS} libchula/test/test_libchula)
  add_test("valgrind-chula-oom" ${VALGRIND} ${VALGRIND_OPTIONS} libchula/test/OOM/oom_libchula)
else()
  message(STATUS "Testing not run under valgrind")
  add_test(hpack test/test_libhpack)
  add_test(chula libchula/test/test_libchula)
  add_test(chula-oom libchula/test/OOM/oom_libchula)
endif()

# config.h
execute_process (COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen-config.py ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE (${CMAKE_CURRENT_BINARY_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h IMMEDIATE)

# libhpack.pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libhpack.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/libhpack.pc @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libhpack.pc
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# CPack
INCLUDE(InstallRequiredSystemLibraries)

set (CPACK_PACKAGE_VERSION_MAJOR ${hpack_SOVERSION_CURRENT})
set (CPACK_PACKAGE_VERSION_MINOR ${hpack_SOVERSION_REVISION})
set (CPACK_PACKAGE_VERSION_PATCH ${hpack_SOVERSION_AGE})
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${hpack_VERSION}")
set (CPACK_PACKAGE_VENDOR "Alvaro Lopez Ortega")
set (CPACK_PACKAGE_CONTACT "alvaro@alobbs.com")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Header Compression for HTTP/2.0 library")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_SOURCE_GENERATOR "TGZ;TBZ2")

list(APPEND CPACK_SOURCE_IGNORE_FILES "/build")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/doc/external")
list(APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.git.*")
list(APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.gdb.*")

INCLUDE(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

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
