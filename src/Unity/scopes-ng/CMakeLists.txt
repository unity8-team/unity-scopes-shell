# Dependencies
include(FindPkgConfig)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.3.1)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
)

set(PLUGIN_SRC
    categories.cpp
    collectors.cpp
    previewmodel.cpp
    previewstack.cpp
    previewwidgetmodel.cpp
    resultsmodel.cpp
    scope.cpp
    scopes.cpp
    utils.cpp
)

add_library(ScopesNG-qml STATIC ${PLUGIN_SRC})
set_target_properties(ScopesNG-qml PROPERTIES COMPILE_FLAGS "-fPIC")

qt5_use_modules(ScopesNG-qml Qml Gui DBus)
