
# Dependencies
pkg_check_modules(SCOPES_API REQUIRED unity-shell-scopes=5)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.6.9)
pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)
pkg_check_modules(UBUNTU_LOCATION_SERVICE REQUIRED ubuntu-location-service)
pkg_check_modules(ONLINE_ACCOUNTS_CLIENT REQUIRED OnlineAccountsClient)

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
)

add_subdirectory(Unity)
add_subdirectory(scope-harness)

