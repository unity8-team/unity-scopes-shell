include(FindPkgConfig)
pkg_check_modules(UNITYCORE REQUIRED unity-core-6.0>=7.1.1)
pkg_check_modules(LIBUNITYPROTO REQUIRED unity-protocol-private)
pkg_check_modules(LIBUNITY REQUIRED unity)
pkg_check_modules(LIBSIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(QTDEE REQUIRED libdee-qt5>=3.2)
pkg_check_modules(SCOPESLIB REQUIRED libunity-scopes>=0.1.3)

set(SCOPES_BIN_DIR ${SCOPESLIB_LIBDIR})
set(SCOPES_TEST_RUNTIME ${CMAKE_CURRENT_BINARY_DIR}/config/Runtime.ini)

add_subdirectory(config)

include_directories(
    ${Unity-qml_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Unity
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIBUNITYPROTO_INCLUDE_DIRS}
    ${LIBUNITY_INCLUDE_DIRS}
    ${LIBSIGCPP_INCLUDE_DIRS}
    ${UNITYCORE_INCLUDE_DIRS}
    ${QTDEE_INCLUDE_DIRS}
    ${SCOPESLIB_INCLUDE_DIRS}
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
        qt5_use_modules(${_test}Exec Test Core Qml)
        set_tests_properties(test${CLASSNAME}${_test}
                PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=minimal;LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/plugins/Unity:${LIBUNITYPROTO_LIBRARY_DIRS};UNITY_SCOPES_RUNTIME_PATH=${SCOPES_TEST_RUNTIME}")
        set_target_properties(${_test}Exec
                PROPERTIES COMPILE_FLAGS "-DTEST_SCOPEREGISTRY_BIN='\"${SCOPES_BIN_DIR}/scoperegistry/scoperegistry\"' -DTEST_RUNTIME_CONFIG='\"${SCOPES_TEST_RUNTIME}\"'")

        target_link_libraries(${_test}Exec
            Unity-qml
            ${UNITYCORE_LDFLAGS}
            ${LIBUNITY_LDFLAGS}
            ${LIBUNITYPROTO_LDFLAGS}
            ${LIBSIGCPP_LDFLAGS}
            ${QTDEE_LDFLAGS}
            ${SCOPESLIB_LDFLAGS}
            )

        set(_test_list "${_test_list};${_test}")
    endforeach()
endmacro(run_tests)

run_tests(
    signalslisttest
    previewbindingstest
    filtersbindingstest
    resultstest
    resultstest-ng
    )
