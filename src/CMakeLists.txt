set(
    SCOPES_SHELL_DEPENDENCIES
    ${SCOPESLIB_LDFLAGS}
    ${GSETTINGSQT_LDFLAGS}
    ${U1DB_LDFLAGS}
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
