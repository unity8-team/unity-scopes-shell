pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1>=0.2 REQUIRED)
pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1>=0.2 REQUIRED)

set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data/")
set(TEST_RUNTIME_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/data/Runtime.ini")

add_subdirectory(data)

add_definitions(
    -DGEOIP_SERVER_BINARY="${CMAKE_CURRENT_SOURCE_DIR}/geoip.ubuntu.com.py"
    -DTEST_DATA_DIR="${TEST_DATA_DIR}"
    -DTEST_RUNTIME_CONFIG="${TEST_RUNTIME_CONFIG}"
    -DTEST_SETTINGS_UNICODE="${CMAKE_CURRENT_SOURCE_DIR}/data/settings-unicode.ini"
    )

include_directories(
    ${CMAKE_SOURCE_DIR}/src/Unity
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${UBUNTU_LOCATION_SERVICE_INCLUDE_DIRS}
    ${QTDBUSTEST_INCLUDE_DIRS}
    ${QTDBUSMOCK_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
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
        add_executable(${_test}Exec
                       ${_test}.cpp
                       ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/PreviewModelInterface.h
                       ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ScopeInterface.h
                       ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/SettingsModelInterface.h
                      )
        qt5_use_modules(${_test}Exec Test Core Qml DBus)
        set_tests_properties(test${CLASSNAME}${_test}
                PROPERTIES
                  RUN_SERIAL TRUE)

        target_link_libraries(${_test}Exec
            Unity-qml
            ${SCOPESLIB_LDFLAGS}
            ${QTDBUSTEST_LIBRARIES}
            ${QTDBUSMOCK_LIBRARIES}
            scope-harness
            )

        set(_test_list "${_test_list};${_test}")
    endforeach()
endmacro(run_tests)

run_tests(
    departmentstest
    favoritestest
    #locationtest
    overviewtest
    previewtest
    resultstest
    settingsendtoendtest
    settingstest
    utilstest
    )

qt5_use_modules(settingstestExec Sql)
