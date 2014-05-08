# Subdirectories
add_subdirectory(OOM)

# Dependencies
pkg_check_modules(CHECK check)

# Testing exec
file(GLOB SRCS *.c)

add_executable (test_libchula ${SRCS})
add_dependencies (test_libchula chula-qa)

link_directories(${CHECK_LIBRARY_DIRS})
include_directories (${CMAKE_SOURCE_DIR} ${CHECK_INCLUDE_DIRS})
target_link_libraries(test_libchula ${CHECK_LDFLAGS} ${CHECK_CFLAGS} chula chula-qa)
