# Macro to check if the _FILE_OFFSET_BITS=64 macro is needed for large files


MACRO(CHECK__FILE_OFFSET_BITS)

  IF(NOT DEFINED _FILE_OFFSET_BITS)
    MESSAGE(STATUS "Checking off_t for large file support")

    # Check if it is supported by default
    TRY_COMPILE(__WITHOUT_FILE_OFFSET_BITS_64
                ${PROJECT_BINARY_DIR}
                ${CMAKE_SOURCE_DIR}/cmake/CheckFileOffset.c)

    # If it's not supported by default check with _FILE_OFFSET_BITS=64 macro
    IF(NOT __WITHOUT_FILE_OFFSET_BITS_64)
      TRY_COMPILE(__WITH_FILE_OFFSET_BITS_64
                  ${PROJECT_BINARY_DIR}
                  ${CMAKE_SOURCE_DIR}/cmake/CheckFileOffset.c)
    ENDIF()

    # We should be able to work with large files one way or another
    IF(NOT __WITHOUT_FILE_OFFSET_BITS_64 AND NOT __WITH_FILE_OFFSET_BITS_64)
      SET(_FILE_OFFSET_BITS "" CACHE INTERNAL "Cannot work with large files with or without _FILE_OFFSET_BITS=64 macro")
      MESSAGE(SEND_ERROR "Cannot work with large files with or without _FILE_OFFSET_BITS=64 macro")

    ELSE()
      IF(__WITHOUT_FILE_OFFSET_BITS_64)
        SET(_FILE_OFFSET_BITS "" CACHE INTERNAL "_FILE_OFFSET_BITS=64 macro is not needed for large files")
      ELSE()
        SET(_FILE_OFFSET_BITS 64 CACHE INTERNAL "_FILE_OFFSET_BITS=64 macro needed for large files")
      ENDIF()

      get_property(DOCSTRING
                   CACHE _FILE_OFFSET_BITS
                   PROPERTY HELPSTRING)
      MESSAGE(STATUS ${DOCSTRING})
    ENDIF()
  ENDIF()

ENDMACRO(CHECK__FILE_OFFSET_BITS)
