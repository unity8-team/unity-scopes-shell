include(FindPkgConfig)
find_package(PythonLibs 3.4 REQUIRED)
find_package(Boost COMPONENTS python-py34 REQUIRED)

include_directories(
        ${PYTHON_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
        ${SCOPESLIB_INCLUDE_DIRS}
)

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra")

add_library(_pyscope_harness SHARED
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
        scope-harness-py.cpp
        registry-py.cpp
        variant-py.cpp
        main.cpp)
target_link_libraries(_pyscope_harness scope-harness ${Boost_LIBRARIES} ${PythonLibs_LIBRARIES})
set_target_properties(_pyscope_harness PROPERTIES PREFIX "" )

add_subdirectory(testing)

set(pyscope_harnessdir "${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages/pyscope_harness")

install(FILES __init__.py DESTINATION ${pyscope_harnessdir})
install(TARGETS _pyscope_harness LIBRARY DESTINATION ${pyscope_harnessdir})
