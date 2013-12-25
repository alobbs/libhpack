set(LIB_NAME hpack)
file(GLOB hpack_SRCS *.c *.h)

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

install (
  TARGETS ${LIB_NAME}
  ARCHIVE DESTINATION lib COMPONENT Development
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
)
