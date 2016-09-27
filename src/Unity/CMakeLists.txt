# export_qmlplugin macro
include(Plugins)

include(QtWithObjectTarget)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SCOPESLIB_INCLUDE_DIRS}
    ${GSETTINGSQT_INCLUDE_DIRS}
    ${U1DB_INCLUDE_DIRS}
)

add_definitions(
    -DUNITY_DLL_EXPORTS=1
    )

set(QMLPLUGIN_SRC
    categories.cpp
    collectors.cpp
    department.cpp
    departmentnode.cpp
    favorites.cpp
    filters.cpp
    filtergroupwidget.cpp
    optionselectorfilter.cpp
    optionselectoroptions.cpp
    rangeinputfilter.cpp
    valuesliderfilter.cpp
    valueslidervalues.cpp
    geoip.cpp
    localization.h
    locationaccesshelper.cpp
    overviewcategories.cpp
    overviewresults.cpp
    overviewscope.cpp
    previewmodel.cpp
    previewwidgetmodel.cpp
    resultsmap.cpp
    resultsmodel.cpp
    scope.cpp
    scopes.cpp
    settingsmodel.cpp
    ubuntulocationservice.cpp
    utils.cpp
    iconutils.cpp
    logintoaccount.cpp
    # We need these headers here so moc runs and we get the moc-stuff
    # compiled in, otherwise we miss some symbols
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/FiltersInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/FilterBaseInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ExpandableFilterWidgetInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/OptionSelectorFilterInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/OptionSelectorOptionsInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/RangeInputFilterInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ValueSliderFilterInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ValueSliderValuesInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/CategoriesInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/NavigationInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/PreviewModelInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/PreviewWidgetModelInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ResultsModelInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ScopeInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/ScopesInterface.h
    ${SCOPES_API_INCLUDEDIR}/unity/shell/scopes/SettingsModelInterface.h
)

# Object code library

add_library(
    Unity-qml-object OBJECT
    ${QMLPLUGIN_SRC}
)

set_target_properties(
    Unity-qml-object
    PROPERTIES
        COMPILE_FLAGS -fPIC
)

# Work around cmake definciency for using Qt with OBJECT target
object_qt5_use_modules(
    Unity-qml-object
    ${SCOPES_SHELL_QT_DEPENDENCIES}
)

# Shared library for plugin

add_library(
  Unity-qml SHARED
  $<TARGET_OBJECTS:Unity-qml-object>
  plugin.cpp
)

target_link_libraries(
    Unity-qml
    ${SCOPES_SHELL_DEPENDENCIES}
)

qt5_use_modules(
    Unity-qml
    ${SCOPES_SHELL_QT_DEPENDENCIES}
)
# Workaround for gcc failure LP: #1417664
set_source_files_properties(previewwidgetmodel.cpp PROPERTIES COMPILE_FLAGS -O1)

# export the qmldir qmltypes and plugin files
export_qmlfiles(Unity Unity)
export_qmlplugin(Unity 0.2 Unity TARGETS Unity-qml)
