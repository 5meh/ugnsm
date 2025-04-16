file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridCellWidgets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridCellWidgets/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridViewManager/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridViewManager/*.h"
)

add_library(UILibrary STATIC ${UI_SOURCES})

target_include_directories(UILibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridCellWidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridViewManager
)

target_link_libraries(UILibrary PUBLIC
    CoreLibrary
    Qt6::Widgets
    Qt6::Gui
)
