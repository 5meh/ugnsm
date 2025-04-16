file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Grid/Management/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Grid/Management/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Information/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Information/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Monitoring/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Monitoring/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/NetworkSortingStrategies/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/NetworkSortingStrategies/*.h"
)

add_library(CoreLibrary STATIC ${CORE_SOURCES})

target_include_directories(CoreLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Grid/Management
    ${CMAKE_CURRENT_SOURCE_DIR}/Network/Information
    ${CMAKE_CURRENT_SOURCE_DIR}/Network/Monitoring
    ${CMAKE_CURRENT_SOURCE_DIR}/Network/NetworkSortingStrategies
)

target_link_libraries(CoreLibrary PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
)
