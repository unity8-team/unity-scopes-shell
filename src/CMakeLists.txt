
include_directories(
    ${UBUNTU_LOCATION_SERVICE_INCLUDE_DIRS}
)

set(
    SCOPES_SHELL_DEPENDENCIES
    ${SCOPESLIB_LDFLAGS}
    ${GSETTINGSQT_LDFLAGS}
    ${U1DB_LDFLAGS}
    ${UBUNTU_LOCATION_SERVICE_LDFLAGS}
    ${ONLINE_ACCOUNTS_CLIENT_LDFLAGS}
)

set(
    SCOPES_SHELL_QT_DEPENDENCIES
    Concurrent
    DBus
    Gui
    Network
    Qml
    Positioning
)

add_subdirectory(Unity)
add_subdirectory(scope-harness)
add_subdirectory(python)
