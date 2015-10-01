# Set up package config.
configure_file(unity-plugin-scopes.pc.in unity-plugin-scopes.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unity-plugin-scopes.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

configure_file(
    "libscope-harness.pc.in"
    "libscope-harness.pc"
    @ONLY
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/libscope-harness.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

add_custom_target(pkgversion ALL COMMAND dpkg-parsechangelog -l${CMAKE_SOURCE_DIR}/debian/changelog --show-field version > ${CMAKE_CURRENT_BINARY_DIR}/version)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version DESTINATION /var/lib/unity-plugin-scopes)
