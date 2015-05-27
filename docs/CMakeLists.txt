find_program(SPHINX_EXEC NAMES sphinx-build DOC "Sphinx documentation generator")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sphinx DEFAULT_MSG
    SPHINX_EXEC
)

add_custom_target(scope_harness_html_docs ALL
    PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../src/python ${SPHINX_EXEC}
        -E
        -b html
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/html")

add_custom_target(scope_harness_json_docs ALL
    PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../src/python ${SPHINX_EXEC}
        -E
        -b json
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/json")

install(
    DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/html
      ${CMAKE_CURRENT_BINARY_DIR}/json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/python3-scope-harness/)
