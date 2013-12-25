find_package(Check REQUIRED)

include_directories (
   ${CMAKE_SOURCE_DIR}
   ${CHECK_INCLUDE_DIRS}
)

set (test_lib_hpack_SRCS
  integer.c
)

link_directories (${CMAKE_BINARY_DIR}/lib_hpack)

add_executable( test_lib_hpack ${test_lib_hpack_SRCS} )
add_dependencies( test_lib_hpack hpack)
target_link_libraries( test_lib_hpack hpack ${CHECK_LIBRARIES})

add_test(test_lib_hpack test_lib_hpack)
