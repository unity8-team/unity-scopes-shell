cmake_minimum_required(VERSION 2.8.9)

project(unity-scopes-shell C CXX)

if(${PROJECT_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
   message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_lower) # Build types should always be lowercase but sometimes they are not.

include(EnableCoverageReport)
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage (or ninja if you use that backend)
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
if(cmake_build_type_lower MATCHES coverage)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage" )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage" )
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage" )
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage" )
  ENABLE_COVERAGE_REPORT(TARGETS ${SHELL_APP} FILTER /usr/include ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_BINARY_DIR}/*)
endif()

# Make sure we have all the needed symbols
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,defs")

# Static C++ checks
add_custom_target(cppcheck COMMAND cppcheck --enable=all -q --error-exitcode=2
                                       ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests)

find_package(PkgConfig)
find_package(Intltool)
find_package(Qt5Concurrent)
find_package(Qt5Core)
find_package(Qt5DBus)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Gui)
find_package(Qt5Positioning)
find_package(Qt5Test)
find_package(Boost COMPONENTS regex REQUIRED)

pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=1.0.5)
pkg_check_modules(SCOPES_API REQUIRED unity-shell-scopes=12)

pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)
pkg_check_modules(UBUNTU_LOCATION_SERVICE REQUIRED ubuntu-location-service)
pkg_check_modules(ONLINE_ACCOUNTS_CLIENT REQUIRED OnlineAccountsClient)

# Standard install paths
include(GNUInstallDirs)

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=plugindir_suffix unity-shell-api OUTPUT_VARIABLE SHELL_PLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(SHELL_PLUGINDIR STREQUAL "")
    set(SHELL_PLUGINDIR ${CMAKE_INSTALL_LIBDIR}/unity8/qml)
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-permissive -pedantic -Wall -Wextra")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "relwithdebinfo")
    option(Werror "Treat warnings as errors" ON)
else()
    option(Werror "Treat warnings as errors" OFF)
endif()

# Check the distribution we are building for, set scope harness version and so accordingly
execute_process(COMMAND lsb_release -c -s OUTPUT_VARIABLE DISTRIBUTION OUTPUT_STRIP_TRAILING_WHITESPACE)
file(READ debian/HARNESS_VERSION HARNESS_FULL_VERSION)
string(STRIP ${HARNESS_FULL_VERSION} HARNESS_FULL_VERSION)

# Parse out the three components.
STRING(REGEX MATCHALL "[0-9]+" full_version_components ${HARNESS_FULL_VERSION})
LIST(GET full_version_components 0 HARNESS_MAJOR)
LIST(GET full_version_components 1 HARNESS_MINOR)
LIST(GET full_version_components 2 HARNESS_MICRO)

set(HARNESS_SO_VERSION ${HARNESS_MAJOR})
if(NOT ${DISTRIBUTION} MATCHES "vivid")
    math(EXPR HARNESS_SO_VERSION "${HARNESS_SO_VERSION} + 1")
endif()

if (Werror)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

# gtk and unity-core (actually sigc++) do not like Qt defining macros named
# "signals" and "slots"
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DVERBOSE_MODEL_UPDATES)

# Translation
set(GETTEXT_PACKAGE "unity-plugin-scopes")
add_definitions(-DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}")
add_definitions(-DLOCALE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/locale")

# Tests
include(CTest)
enable_testing()
ADD_CUSTOM_TARGET(
    check
    ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
)

# add subdirectories to build
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(data)
#add_subdirectory(tools)
add_subdirectory(po)
add_subdirectory(docs)
