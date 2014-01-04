find_package(Check REQUIRED)

include_directories (
   ${CMAKE_SOURCE_DIR}
   ${CHECK_INCLUDE_DIRS}
)

file(GLOB SRCS *.c)

add_executable (chula_test ${SRCS})
add_dependencies (chula_test chula)
target_link_libraries (chula_test chula ${CHECK_LIBRARIES})

if(VALGRIND)
  foreach(target ${ALL_TESTCASES})
    add_test("valgrind-${target}" ${VALGRIND} --error-exitcode=255 --leak-check=full ${TEST_BIN_DIR}/${target})
  endforeach()
endif()
