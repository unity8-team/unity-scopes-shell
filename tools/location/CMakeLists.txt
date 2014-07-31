
pkg_check_modules(UBUNTU_LOCATION_SERVICE REQUIRED ubuntu-location-service)

include_directories(
    ${Unity-qml_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Unity
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    ${UBUNTU_LOCATION_SERVICE_INCLUDE_DIRS}
    )

add_executable(location
    main.cpp
    )

target_link_libraries(location
    Unity-qml
    ${SCOPESLIB_LDFLAGS}
    ${GSETTINGSQT_LDFLAGS}
    )