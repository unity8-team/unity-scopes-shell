
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPERUNNER_BIN "" --variable=scoperunner_bin )
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPEREGISTRY_BIN "" --variable=scoperegistry_bin )

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    )
    
add_definitions(
    -DSCOPESLIB_SCOPEREGISTRY_BIN="${SCOPESLIB_SCOPEREGISTRY_BIN}"
    -DDEB_HOST_MULTIARCH="${CMAKE_LIBRARY_ARCHITECTURE}"
    )

add_library(
    scope-harness SHARED
    matcher/category-matcher.cpp
    matcher/category-list-matcher.cpp
    matcher/match-result.cpp
    matcher/result-matcher.cpp
    registry/pre-existing-registry.cpp
    view/preview-view.cpp
    view/results-view.cpp
    category.cpp
    result.cpp
    scope-harness.cpp
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
