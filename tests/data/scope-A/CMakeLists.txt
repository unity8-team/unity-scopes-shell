include(FindPkgConfig)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.1.2)

set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})

include_directories(${SCOPESLIB_INCLUDE_DIRS})

add_library(scope-A MODULE scope-A.cpp)
target_link_libraries(scope-A ${SCOPESLIB_LDFLAGS})

configure_file(scope-A.ini.in scope-A.ini)
