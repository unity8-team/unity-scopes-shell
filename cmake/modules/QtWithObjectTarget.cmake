
function(object_qt5_use_modules TARGET_NAME)
    foreach(_module ${ARGN})
        target_include_directories(
            ${TARGET_NAME} PRIVATE
            $<TARGET_PROPERTY:Qt5::${_module},INTERFACE_INCLUDE_DIRECTORIES>
        )
        target_compile_definitions(
            ${TARGET_NAME} PRIVATE
            $<TARGET_PROPERTY:Qt5::${_module},INTERFACE_COMPILE_DEFINITIONS>
        )
    endforeach()
endfunction()