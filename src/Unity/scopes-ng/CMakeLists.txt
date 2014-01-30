# Dependencies
include(FindPkgConfig)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.2.8)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Qml_INCLUDE_DIRS}
    ${SCOPESLIB_INCLUDE_DIRS}
)

set(PLUGIN_SRC
    categories.cpp
    collectors.cpp
    preview.cpp
    resultsmodel.cpp
    scope.cpp
    scopes.cpp
    utils.cpp
)

add_library(ScopesNG-qml OBJECT ${PLUGIN_SRC})
set_target_properties(ScopesNG-qml PROPERTIES COMPILE_FLAGS "-fPIC")
