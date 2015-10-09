include_directories(${SCOPESLIB_INCLUDE_DIRS})

add_library(mock-scope-manyresults MODULE mock-scope-manyresults.cpp)
target_link_libraries(mock-scope-manyresults ${SCOPESLIB_LDFLAGS})

configure_file(mock-scope-manyresults.ini.in mock-scope-manyresults.ini)
