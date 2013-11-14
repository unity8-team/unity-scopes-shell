# Set up package config.
configure_file(unity-scopes-qml.pc.in unity-scopes-qml.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unity-scopes-qml.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
