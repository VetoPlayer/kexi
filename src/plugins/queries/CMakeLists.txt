include_directories(
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/widget
    ${CMAKE_SOURCE_DIR}/src/widget/tableview
    ${CMAKE_SOURCE_DIR}/src/widget/properties
    ${CMAKE_SOURCE_DIR}/src/kexiutils/style
)

# the main plugin
set(kexi_queryplugin_SRCS
   kexiquerypart.cpp
   kexiquerydesignersql.cpp
   kexiquerydesignerguieditor.cpp
   kexiqueryview.cpp
)

add_library(kexi_queryplugin MODULE ${kexi_queryplugin_SRCS})
kcoreaddons_desktop_to_json(kexi_queryplugin kexi_queryplugin.desktop)

target_link_libraries(kexi_queryplugin
    PRIVATE
        kexiextendedwidgets
        kexidataviewcommon
        kexidatatable
        kexirelationsview

        Qt5::Xml
)

install(TARGETS kexi_queryplugin  DESTINATION ${KEXI_PLUGIN_INSTALL_DIR})
