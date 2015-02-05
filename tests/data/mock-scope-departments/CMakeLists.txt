pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.4.0)

set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SCOPE_SOURCES
    mock-scope-departments.cpp
    )

add_library(mock-scope-departments MODULE ${SCOPE_SOURCES})
target_link_libraries(mock-scope-departments ${SCOPESLIB_LDFLAGS})

configure_file(mock-scope-departments.ini.in mock-scope-departments.ini)
configure_file(mock-scope-departments-settings.ini mock-scope-departments-settings.ini)
