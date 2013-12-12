add_subdirectory(mock-scope)

configure_file(Runtime.ini.in Runtime.ini @ONLY)
configure_file(Registry.ini.in Registry.ini @ONLY)
configure_file(Zmq.ini.in Zmq.ini @ONLY)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/endpoints)
