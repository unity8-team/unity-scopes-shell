add_subdirectory(mock-scope)
add_subdirectory(mock-scope-departments)
add_subdirectory(mock-scope-double-nav)
add_subdirectory(mock-scope-info)
add_subdirectory(mock-scope-ttl)
add_subdirectory(scopes)

configure_file(Runtime.ini.in Runtime.ini @ONLY)
configure_file(Registry.ini.in Registry.ini @ONLY)
configure_file(Zmq.ini.in Zmq.ini @ONLY)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/endpoints)
