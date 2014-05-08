# Testing exec
file(GLOB SRCS *.c)

add_executable (oom_libchula ${SRCS})
add_dependencies (oom_libchula chula chula-qa)

include_directories (${CMAKE_SOURCE_DIR})
target_link_libraries(oom_libchula chula chula-qa)
