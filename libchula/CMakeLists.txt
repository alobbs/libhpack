# Subdirectories
add_subdirectory(test)

if(BUILD_DOCS)
  add_subdirectory(doc)
endif(BUILD_DOCS)

# Library
set(LIB_NAME chula)
file(GLOB chula_SRCS *.c *.h)
list(APPEND chula_SRCS chula-ret.h)

file(GLOB chula_HDRS *.h)
file(GLOB chula_HDRS_wo *.h)
list(APPEND chula_HDRS chula-ret.h)
list(REMOVE_ITEM chula_HDRS_wo chula-ret.h)

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
    COMPILE_FLAGS -DCHULA_COMPILATION
)

# Auto-ret header
add_custom_command (
    OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/chula-ret.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../tools/auto-ret.py
    ARGS    --path ${CMAKE_CURRENT_SOURCE_DIR} --output ${CMAKE_CURRENT_SOURCE_DIR}/chula-ret.h --suffix _RET
    DEPENDS ${chula_HDRS_wo}
    COMMENT "Generating the chula-ret.h file..."
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
