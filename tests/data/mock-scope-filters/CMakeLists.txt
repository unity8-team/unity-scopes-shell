pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.6.15)

set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SCOPE_SOURCES
    mock-scope-filters.cpp
    )

add_library(mock-scope-filters MODULE ${SCOPE_SOURCES})
target_link_libraries(mock-scope-filters ${SCOPESLIB_LDFLAGS})

configure_file(mock-scope-filters.ini.in mock-scope-filters.ini)
