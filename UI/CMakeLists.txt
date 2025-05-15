file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridCellWidgets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridCellWidgets/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridViewManager/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridViewManager/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/DialogWindows/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components/DialogWindows/*.h"
)

add_library(UILibrary STATIC ${UI_SOURCES})

target_include_directories(UILibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridCellWidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/Grid/GridViewManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/DialogWindows
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core  # For GridManager
)

target_link_libraries(UILibrary PUBLIC
    CoreLibrary
    UtilitiesLibrary
    Qt6::Widgets
    Qt6::Gui
)
