set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SCOPE_SOURCES
    mock-scope-info.cpp
    )

add_library(mock-scope-info MODULE ${SCOPE_SOURCES})
target_link_libraries(mock-scope-info ${SCOPESLIB_LDFLAGS})

configure_file(mock-scope-info.ini.in mock-scope-info.ini)
