# Dependencies
pkg_check_modules(CHECK check)

# Library
set(LIB_NAME chula-qa)
file(GLOB chula_qa_SRCS *.c *.h)
file(GLOB chula_qa_HDRS *.h)

add_library (
    ${LIB_NAME}
    STATIC
    ${chula_qa_SRCS}
)

set_target_properties (
    ${LIB_NAME}
    PROPERTIES
    VERSION ${chula_VERSION}
    SOVERSION ${chula_SOVERSION}
    COMPILE_FLAGS -DCHULA_QA_COMPILATION
)

add_dependencies(${LIB_NAME} libchula)

if (UNIX AND NOT APPLE)
    set_target_properties (
        ${LIB_NAME}
        PROPERTIES
        LINK_FLAGS "-Wl,--wrap=malloc -Wl,--wrap=realloc -Wl,--wrap=free"
    )
endif()

link_directories(${CHECK_LIBRARY_DIRS})
include_directories (${CMAKE_SOURCE_DIR} ${CHECK_INCLUDE_DIRS})
target_link_libraries (${LIB_NAME} ${CHECK_LDFLAGS} ${CHECK_CFLAGS})

# Installation
install (
  TARGETS ${LIB_NAME}
  ARCHIVE DESTINATION lib COMPONENT Development
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
)

install (
  FILES ${chula_qa_HDRS}
  DESTINATION include/libchula-qa
)
