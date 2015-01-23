add_subdirectory(pyscope_harness)
add_test(python_bindings ${CMAKE_CURRENT_BINARY_DIR}/python-test.py ${PROJECT_BINARY_DIR}/tests/data)
