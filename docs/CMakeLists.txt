find_program(SPHINX_EXEC NAMES sphinx-build DOC "Sphinx documentation generator")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sphinx DEFAULT_MSG
    SPHINX_EXEC
)

add_custom_target(scope_harness_docs ALL
    PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../src/python ${SPHINX_EXEC}
        -vv
        -b html
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}")
