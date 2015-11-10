include(FindPkgConfig)

# Ubuntu xenial needs python 3.5
if(${DISTRIBUTION} MATCHES "vivid" OR ${DISTRIBUTION} MATCHES "wily")
    find_package(PythonLibs 3.4 REQUIRED)
    find_package(Boost COMPONENTS python-py34 REQUIRED)
else()
    find_package(PythonLibs 3.5 REQUIRED)
    find_package(Boost COMPONENTS python-py35 REQUIRED)
endif()

include_directories(
        ${PYTHON_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
        ${SCOPESLIB_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra")

add_library(_scope_harness SHARED
        exceptions-py.cpp
        category-py.cpp
        department-py.cpp
        matchers-py.cpp
        preview-widget-py.cpp
        preview-widget-list-py.cpp
        abstract-view-py.cpp
        preview-view-py.cpp
        preview-matchers-py.cpp
        results-view-py.cpp
        result-py.cpp
        settings-view-py.cpp
        settings-matchers-py.cpp
        scope-harness-py.cpp
        scope-uri-py.cpp
        registry-py.cpp
        variant-py.cpp
        main.cpp)
target_link_libraries(_scope_harness scope-harness ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
set_target_properties(_scope_harness PROPERTIES PREFIX "" )

set(scope_harnessdir "${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages/scope_harness")

install(FILES __init__.py DESTINATION ${scope_harnessdir})
install(FILES testing.py DESTINATION ${scope_harnessdir})
install(TARGETS _scope_harness LIBRARY DESTINATION ${scope_harnessdir})

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)
configure_file(testing.py ${CMAKE_CURRENT_BINARY_DIR}/testing.py COPYONLY)
