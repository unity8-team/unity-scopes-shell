
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPERUNNER_BIN "" --variable=scoperunner_bin )
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPEREGISTRY_BIN "" --variable=scoperegistry_bin )

include_directories(
    ${CMAKE_SOURCE_DIR}/src/Unity
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    )
    
add_definitions(
    -DSCOPESLIB_SCOPEREGISTRY_BIN="${SCOPESLIB_SCOPEREGISTRY_BIN}"
    )

add_library(
    scope-harness SHARED
    registry-spawner.cpp
    test-utils.cpp
    )

target_link_libraries(
    scope-harness
    Unity-qml
    ${GSETTINGSQT_LDFLAGS}
    )

qt5_use_modules(
    scope-harness
    Network
    Test
    )
