set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/componentregistry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/componentregistry.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Grid/Managment/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Grid/Managment/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Information/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Information/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Monitoring/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/Monitoring/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/NetworkSortingStrategies/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Network/NetworkSortingStrategies/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/TaskSystem/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/TaskSystem/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/*.cpp"
)

add_library(CoreLibrary STATIC ${CORE_SOURCES})

target_include_directories(CoreLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Grid/Managment
    ${CMAKE_CURRENT_SOURCE_DIR}/Network/Information
    ${CMAKE_CURRENT_SOURCE_DIR}/Network/Monitoring
    ${CMAKE_CURRENT_SOURCE_DIR}/Network/NetworkSortingStrategies
    ${CMAKE_CURRENT_SOURCE_DIR}/TaskSystem/
    ${CMAKE_CURRENT_SOURCE_DIR}/Settings/
    ${CMAKE_CURRENT_SOURCE_DIR}/../Utilities
)

target_link_libraries(CoreLibrary PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    Qt6::Gui
    UtilitiesLibrary
)
