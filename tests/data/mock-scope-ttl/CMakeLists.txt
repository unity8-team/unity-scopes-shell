include(FindPkgConfig)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.4.0)

set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SCOPE_SOURCES
    mock-scope-ttl.cpp
    )

qt5_add_dbus_interface(
    SCOPE_SOURCES
    ${TEST_INTERFACE_XML}
    TestInterface
    )

add_library(mock-scope-ttl MODULE ${SCOPE_SOURCES})
target_link_libraries(mock-scope-ttl ${SCOPESLIB_LDFLAGS})

qt5_use_modules(mock-scope-ttl
    Core
    DBus
    )

configure_file(mock-scope-ttl.ini.in mock-scope-ttl.ini)
