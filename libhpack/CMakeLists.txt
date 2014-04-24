# Library
set(LIB_NAME hpack)
file(GLOB hpack_SRCS *.c *.h)
LIST (APPEND hpack_SRCS huffman_tables.c)
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

add_custom_command (
    OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/huffman_tables.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/huffman-gen.py
    ARGS    -v -o ${CMAKE_CURRENT_SOURCE_DIR}/huffman_tables.c
    COMMENT "Generating the huffman_tables.c file..."
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

# libhpack.pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libhpack.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/libhpack.pc @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libhpack.pc
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
