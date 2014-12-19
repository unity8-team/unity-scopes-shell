
set(API_VERSION 1)
set(ABI_VERSION 1)

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

# Scope harness library

add_library(
    scope-harness SHARED
    $<TARGET_OBJECTS:Unity-qml-object>
    internal/signal-handler.cpp
    matcher/category-matcher.cpp
    matcher/category-list-matcher.cpp
    matcher/match-result.cpp
    matcher/preview-matcher.cpp
    matcher/preview-column-matcher.cpp
    matcher/preview-widget-matcher.cpp
    matcher/result-matcher.cpp
    preview/preview-widget.cpp
    registry/custom-registry.cpp
    registry/pre-existing-registry.cpp
    registry/system-registry.cpp
    results/category.cpp
    results/result.cpp
    view/preview-view.cpp
    view/results-view.cpp
    scope-harness.cpp
    test-utils.cpp
)

target_link_libraries(
    scope-harness
    ${SCOPES_SHELL_DEPENDENCIES}
)

qt5_use_modules(
    scope-harness
    Test
    ${SCOPES_SHELL_QT_DEPENDENCIES}
)

set_target_properties(
    scope-harness
    PROPERTIES
    VERSION ${API_VERSION}.0.0
    SOVERSION ${ABI_VERSION}
    PUBLIC_HEADER "${SCOPE_HARNESS_PUBLIC_HEADERS}"
)

install(
    TARGETS scope-harness
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h"
    PATTERN "internal" EXCLUDE
)

# Custom registry executable

add_executable(
    custom-registry
    registry/custom-registry-main.cpp
)

target_link_libraries(
    custom-registry
    scope-harness
)
