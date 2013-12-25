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

# Includes
include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckFunctionExists)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Library source code
add_subdirectory(lib_hpack)

if(BUILD_TESTS)
include(CTest)
enable_testing(true)
add_subdirectory(test)
endif(BUILD_TESTS)

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
