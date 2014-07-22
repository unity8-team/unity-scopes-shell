pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.5.0)

include_directories(${SCOPESLIB_INCLUDE_DIRS})

add_library(scopes MODULE scopes.cpp)
target_link_libraries(scopes ${SCOPESLIB_LDFLAGS})

configure_file(scopes.ini.in scopes.ini)
