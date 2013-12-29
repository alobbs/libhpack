set(LIB_NAME chula)
file(GLOB chula_SRCS *.c *.h)

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

install (
  TARGETS ${LIB_NAME}
  ARCHIVE DESTINATION lib COMPONENT Development
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
)

if(BUILD_TESTS)
  add_subdirectory(test)
endif(BUILD_TESTS)
