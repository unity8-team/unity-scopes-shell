
# Dependencies
pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)
pkg_check_modules(UBUNTU_LOCATION_SERVICE REQUIRED ubuntu-location-service)
pkg_check_modules(ONLINE_ACCOUNTS_CLIENT REQUIRED OnlineAccountsClient)

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
)

add_subdirectory(Unity)
add_subdirectory(scope-harness)
add_subdirectory(python)

