set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SCOPE_SOURCES
    mock-scope-departments-flipflop.cpp
    )

add_library(mock-scope-departments-flipflop MODULE ${SCOPE_SOURCES})
target_link_libraries(mock-scope-departments-flipflop ${SCOPESLIB_LDFLAGS})

configure_file(mock-scope-departments-flipflop.ini.in mock-scope-departments-flipflop.ini)
