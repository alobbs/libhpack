find_package(Sphinx REQUIRED)
find_program(DOXYGEN doxygen)

add_custom_target(doxygen_chula
    COMMAND doxygen doxygen.conf
    COMMENT "Build doxygen xml files used by sphinx/breathe."
)

# configured documentation tools and intermediate build results
set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
set(SPHINX_HTML_DIR  "${CMAKE_CURRENT_BINARY_DIR}/html")

# Configuration files
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${BINARY_BUILD_DIR}/conf.py"
    @ONLY
)

configure_file(
   doxygen.conf.in
   doxygen.conf
   @ONLY
)

add_custom_target(html_docs_chula
    ${SPHINX_EXECUTABLE}
    -b html
    -c "${BINARY_BUILD_DIR}"
    -d "${SPHINX_CACHE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SPHINX_HTML_DIR}"
    DEPENDS doxygen_chula
    COMMENT "Building HTML documentation with Sphinx"
)

add_dependencies(doc DEPENDS html_docs_chula)
