
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPERUNNER_BIN "" --variable=scoperunner_bin )
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPEREGISTRY_BIN "" --variable=scoperegistry_bin )
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPESDIR "" --variable=scopesdir )

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    )
    
add_definitions(
    -DSCOPESLIB_SCOPERUNNER_BIN="${SCOPESLIB_SCOPERUNNER_BIN}"
    -DSCOPESLIB_SCOPEREGISTRY_BIN="${SCOPESLIB_SCOPEREGISTRY_BIN}"
    -DSCOPESLIB_SCOPESDIR="${SCOPESLIB_SCOPESDIR}"
    -DDEB_HOST_MULTIARCH="${CMAKE_LIBRARY_ARCHITECTURE}"
    )

add_library(
    scope-harness SHARED
    internal/signal-handler.cpp
    matcher/category-matcher.cpp
    matcher/category-list-matcher.cpp
    matcher/match-result.cpp
    matcher/result-matcher.cpp
    registry/custom-registry.cpp
    registry/pre-existing-registry.cpp
    registry/system-registry.cpp
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

add_executable(
    custom-registry
    registry/custom-registry-main.cpp
)

target_link_libraries(
    custom-registry
    scope-harness
    )