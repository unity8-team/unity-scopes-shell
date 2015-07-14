set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SCOPE_SOURCES
    mock-scope-double-nav.cpp
    )

add_library(mock-scope-double-nav MODULE ${SCOPE_SOURCES})
target_link_libraries(mock-scope-double-nav ${SCOPESLIB_LDFLAGS})

configure_file(mock-scope-double-nav.ini.in mock-scope-double-nav.ini)
