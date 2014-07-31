pkg_check_modules(SCOPES_API REQUIRED unity-shell-scopes=3)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.5.3)
pkg_check_modules(GSETTINGSQT REQUIRED gsettings-qt)
pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1>=0.2 REQUIRED)
pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1>=0.2 REQUIRED)

_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPERUNNER_BIN "" --variable=scoperunner_bin )
_pkgconfig_invoke(libunity-scopes SCOPESLIB SCOPEREGISTRY_BIN "" --variable=scoperegistry_bin )

set(SCOPES_TEST_RUNTIME ${CMAKE_CURRENT_BINARY_DIR}/data/Runtime.ini)

add_subdirectory(data)

add_definitions(-DGEOIP_SERVER_BINARY="${CMAKE_CURRENT_SOURCE_DIR}/geoip.ubuntu.com.py")

include_directories(
    ${Unity-qml_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Unity
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    ${UBUNTU_LOCATION_SERVICE_INCLUDE_DIRS}
    ${QTDBUSTEST_INCLUDE_DIRS}
    ${QTDBUSMOCK_INCLUDE_DIRS}
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
                      )
        qt5_use_modules(${_test}Exec Test Core Qml DBus)
        set_tests_properties(test${CLASSNAME}${_test}
                PROPERTIES ENVIRONMENT "UNITY_SCOPES_LIST_DELAY=5;UNITY_SCOPES_RESULTS_TTL_OVERRIDE=250" RUN_SERIAL TRUE)
        set_target_properties(${_test}Exec
                PROPERTIES COMPILE_FLAGS "-DTEST_SCOPEREGISTRY_BIN='\"${SCOPESLIB_SCOPEREGISTRY_BIN}\"' -DTEST_RUNTIME_CONFIG='\"${SCOPES_TEST_RUNTIME}\"'")

        target_link_libraries(${_test}Exec
            Unity-qml
            ${SCOPESLIB_LDFLAGS}
            ${GSETTINGSQT_LDFLAGS}
            ${QTDBUSTEST_LIBRARIES}
            ${QTDBUSMOCK_LIBRARIES}
            )

        set(_test_list "${_test_list};${_test}")
    endforeach()
endmacro(run_tests)

run_tests(
    departmentstest
    locationtest
    overviewtest
    previewtest
    resultstest
    settingstest
    utilstest
    )

qt5_use_modules(settingstestExec Sql)
