# Dependencies
include(FindPkgConfig)
pkg_check_modules(QTDEE REQUIRED libdee-qt5>=3.2)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QTDEE_INCLUDE_DIRS}
    ${Qt5Qml_INCLUDE_DIRS}
)

set(PLUGIN_SRC
    scope.cpp
    scopes.cpp
)

add_library(ScopesNG-qml OBJECT ${PLUGIN_SRC})
set_target_properties(ScopesNG-qml PROPERTIES COMPILE_FLAGS "-fPIC")
