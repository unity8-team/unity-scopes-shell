include(FindPkgConfig)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.4.0)
pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)
find_package(Qt5DBus)

_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPERUNNER_BIN "" --variable=scoperunner_bin )
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPEREGISTRY_BIN "" --variable=scoperegistry_bin )

set(SCOPES_TEST_RUNTIME ${CMAKE_CURRENT_BINARY_DIR}/data/Runtime.ini)
set(TEST_INTERFACE_XML "${CMAKE_CURRENT_SOURCE_DIR}/data/com.canonical.scopes.test.xml")

add_subdirectory(data)

include_directories(
    ${Unity-qml_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Unity
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    )

set(TEST_INTERFACE_SOURCES
    scope-test-interface.cpp
    )

qt5_add_dbus_adaptor(
    TEST_INTERFACE_SOURCES
    ${TEST_INTERFACE_XML}
    tests/scope-test-interface.h
    ScopeTestInterface
    TestInterfaceAdaptor
)

add_library(
    test-interface
    STATIC
    ${TEST_INTERFACE_SOURCES}
    )
    
qt5_use_modules(test-interface
    Core
    DBus
    Test
    )

macro(run_tests)
    set(_test_list "")
    foreach(_test ${ARGN})
        set(testCommand
                ${CMAKE_CURRENT_BINARY_DIR}/${_test}Exec
                -o ${CMAKE_BINARY_DIR}/${_test}.xml,xunitxml
                -o -,txt)
        add_test(NAME test${CLASSNAME}${_test} COMMAND ${testCommand})
        add_custom_target(${_test} ${testCommand})
        add_executable(${_test}Exec ${_test}.cpp)
        qt5_use_modules(${_test}Exec Test Core Qml DBus)
        set_tests_properties(test${CLASSNAME}${_test}
                PROPERTIES ENVIRONMENT "UNITY_SCOPES_LIST_DELAY=5;UNITY_SCOPES_RESULTS_TTL_OVERRIDE=250" RUN_SERIAL TRUE)
        set_target_properties(${_test}Exec
                PROPERTIES COMPILE_FLAGS "-DTEST_SCOPEREGISTRY_BIN='\"${SCOPESLIB_SCOPEREGISTRY_BIN}\"' -DTEST_RUNTIME_CONFIG='\"${SCOPES_TEST_RUNTIME}\"'")

        target_link_libraries(${_test}Exec
            Unity-qml
            ${SCOPESLIB_LDFLAGS}
            ${GSETTINGSQT_LDFLAGS}
            )

        set(_test_list "${_test_list};${_test}")
    endforeach()
endmacro(run_tests)

run_tests(
    resultstest
    previewtest
    utilstest
    )

target_link_libraries(resultstestExec
    test-interface
    )