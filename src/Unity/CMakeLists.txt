# export_qmlplugin macro
include(Plugins)

# Dependencies
include(FindPkgConfig)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.4.0)
pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
)

set(QMLPLUGIN_SRC
    categories.cpp
    collectors.cpp
    previewmodel.cpp
    previewstack.cpp
    previewwidgetmodel.cpp
    resultsmodel.cpp
    scope.cpp
    scopes.cpp
    utils.cpp
    plugin.cpp
    iconutils.cpp
)

add_library(Unity-qml SHARED ${QMLPLUGIN_SRC})

target_link_libraries(Unity-qml
    ${SCOPESLIB_LDFLAGS}
    ${GSETTINGSQT_LDFLAGS}
    )

qt5_use_modules(Unity-qml Qml Gui DBus)

# export the qmldir qmltypes and plugin files
export_qmlfiles(Unity Unity)
export_qmlplugin(Unity 0.2 Unity TARGETS Unity-qml)
