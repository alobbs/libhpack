# Library
set(LIB_NAME hpack)
file(GLOB hpack_SRCS *.c *.h)
file(GLOB hpack_HDRS *.h)

add_library (
    ${LIB_NAME}
    STATIC
    ${hpack_SRCS}
)

set_target_properties (
    ${LIB_NAME}
    PROPERTIES
    VERSION ${hpack_VERSION}
    SOVERSION ${hpack_SOVERSION}
)

target_link_libraries (${LIB_NAME} ${LIBM})

# Installation
install (
  TARGETS ${LIB_NAME}
  ARCHIVE DESTINATION lib COMPONENT Development
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
)

install (
  FILES ${hpack_HDRS}
  DESTINATION include/libhpack
)
