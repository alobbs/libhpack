find_package(Check REQUIRED)

include_directories (
   ${CMAKE_SOURCE_DIR}
   ${CHECK_INCLUDE_DIRS}
)

set (test_libhpack_SRCS
  integer.c
)

link_directories (${CMAKE_BINARY_DIR}/libhpack)

add_executable( test_libhpack ${test_libhpack_SRCS} )
add_dependencies( test_libhpack hpack)
target_link_libraries( test_libhpack hpack ${CHECK_LIBRARIES})

add_test(test_libhpack test_libhpack)
