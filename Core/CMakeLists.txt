file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Grid/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Grid/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/NetworkSortingStrategies/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/NetworkSortingStrategies/*.cpp"
)

add_library(CoreLibrary STATIC ${CORE_SOURCES})

target_include_directories(CoreLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Grid
    ${CMAKE_CURRENT_SOURCE_DIR}/Network
    ${CMAKE_CURRENT_SOURCE_DIR}/NetworkSortingStrategies
)

target_link_libraries(CoreLibrary PUBLIC Qt6::Core Qt6::Network Qt6::Widgets UILibrary)#TODO:mb remove UILibrary
