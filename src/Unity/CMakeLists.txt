# export_qmlplugin macro
include(Plugins)

# Dependencies
pkg_check_modules(SCOPES_API REQUIRED unity-shell-scopes=6)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.6.12)
pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)
pkg_check_modules(UBUNTU_LOCATION_SERVICE REQUIRED ubuntu-location-service)
pkg_check_modules(ONLINE_ACCOUNTS_CLIENT REQUIRED OnlineAccountsClient)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    ${U1DB_INCLUDE_DIRS}
    ${UBUNTU_LOCATION_SERVICE_INCLUDE_DIRS}
)

set(QMLPLUGIN_SRC
    categories.cpp
    collectors.cpp
    department.cpp
    departmentnode.cpp
    geoip.cpp
    localization.h
    locationservice.cpp
    overviewcategories.cpp
    overviewresults.cpp
    overviewscope.cpp
    previewmodel.cpp
    previewstack.cpp
    previewwidgetmodel.cpp
    resultsmodel.cpp
    scope.cpp
    scopes.cpp
    settingsmodel.cpp
    ubuntulocationservice.cpp
    utils.cpp
    plugin.cpp
    iconutils.cpp
    # We need these headers here so moc runs and we get the moc-stuff
    # compiled in, otherwise we miss some symbols
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/CategoriesInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/NavigationInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/PreviewModelInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/PreviewStackInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/PreviewWidgetModelInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ResultsModelInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ScopeInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ScopesInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/SettingsModelInterface.h
)

add_library(Unity-qml SHARED ${QMLPLUGIN_SRC})

target_link_libraries(Unity-qml
    ${SCOPESLIB_LDFLAGS}
    ${GSETTINGSQT_LDFLAGS}
    ${U1DB_LDFLAGS}
    ${UBUNTU_LOCATION_SERVICE_LDFLAGS}
    ${ONLINE_ACCOUNTS_CLIENT_LDFLAGS}
    )

qt5_use_modules(Unity-qml Qml Gui DBus Concurrent)

# export the qmldir qmltypes and plugin files
export_qmlfiles(Unity Unity)
export_qmlplugin(Unity 0.2 Unity TARGETS Unity-qml)
