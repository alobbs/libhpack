# Subdirectories
if(BUILD_TESTS)
  add_subdirectory(test)
endif(BUILD_TESTS)

if(BUILD_DOCS)
  add_subdirectory(doc)
endif(BUILD_DOCS)

# Library
set(LIB_NAME chula)
file(GLOB chula_SRCS *.c *.h)
file(GLOB chula_HDRS *.h)

add_library (
    ${LIB_NAME}
    STATIC
    ${chula_SRCS}
)

set_target_properties (
    ${LIB_NAME}
    PROPERTIES
    VERSION ${chula_VERSION}
    SOVERSION ${chula_SOVERSION}
)

# Installation
install (
  TARGETS ${LIB_NAME}
  ARCHIVE DESTINATION lib COMPONENT Development
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
)

install (
  FILES ${chula_HDRS}
  DESTINATION include/libchula
)

# libchula.pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libchula.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/libchula.pc @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libchula.pc
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
