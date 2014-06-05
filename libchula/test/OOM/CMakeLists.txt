# Testing exec
file(GLOB SRCS *.c)

add_executable (oom_libchula ${SRCS})
add_dependencies (oom_libchula chula chula-qa)

include_directories (${CMAKE_SOURCE_DIR})
target_link_libraries(oom_libchula chula chula-qa)

if (UNIX AND NOT APPLE)
    set_target_properties (
        oom_libchula
        PROPERTIES
        LINK_FLAGS "-Wl,--wrap=malloc -Wl,--wrap=realloc -Wl,--wrap=free"
    )
endif()
