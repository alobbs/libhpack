pkg_check_modules(CHECK check)

file(GLOB SRCS *.c)

add_executable (chula_test ${SRCS})
add_dependencies (chula_test chula)

link_directories(${CHECK_LIBRARY_DIRS})
include_directories (${CMAKE_SOURCE_DIR} ${CHECK_INCLUDE_DIRS})
target_link_libraries(chula_test ${CHECK_LDFLAGS} ${CHECK_CFLAGS} chula)
