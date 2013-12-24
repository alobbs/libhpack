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

### INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
###
### set(LIB_NAME hpack)
### file(GLOB SRC_LIBRARY *.c *.h)
###
### add_library(
###     ${LIB_NAME}
###     SHARED
###     ${SRC_LIBRARY}
###     ${INCLUDE_FILES}
### )
###
### set_target_properties(
###     ${LIB_NAME}
###     PROPERTIES
###     VERSION ${hpack_VERSION}
###     SOVERSION ${hpack_SOVERSION}
### )
###
#target_link_libraries (
#    hpack
#)
###
### install(
###   TARGETS ${LIB_NAME}
###   ARCHIVE DESTINATION lib COMPONENT Development
###   LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
### )
