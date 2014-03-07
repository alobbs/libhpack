find_package(PkgConfig REQUIRED)
pkg_check_modules(CHECK REQUIRED check)

file(GLOB SRCS *.c)

add_executable (test_libhpack ${SRCS})
add_dependencies (test_libhpack hpack)

include_directories (${CMAKE_SOURCE_DIR} ${CHECK_INCLUDE_DIRS})
link_directories (${CMAKE_BINARY_DIR}/libhpack ${CHECK_LIBRARY_DIRS})
target_link_libraries(test_libhpack ${CHECK_LDFLAGS} ${CHECK_CFLAGS} chula hpack)
