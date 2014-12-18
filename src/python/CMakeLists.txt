include(FindPkgConfig)
find_package(PythonLibs 3.4 REQUIRED)
find_package(Boost COMPONENTS python-py34 REQUIRED)

include_directories(
        ${PYTHON_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
        ${SCOPESLIB_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra")

add_library(harnesspy SHARED results-view-py.cpp scope-harness-py.cpp registry-py.cpp main.cpp)
target_link_libraries(harnesspy scope-harness ${Boost_LIBRARIES} ${PythonLibs_LIBRARIES})
set_target_properties(harnesspy PROPERTIES PREFIX "" )
