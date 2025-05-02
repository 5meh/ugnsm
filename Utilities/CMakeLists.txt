set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE UTILS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Parser/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Parser/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/LedIndicator/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LedIndicator/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Delegates/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Delegates/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Logger/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Logger/*.cpp"
)

add_library(UtilitiesLibrary STATIC ${UTILS_SOURCES})

target_include_directories(UtilitiesLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser
    ${CMAKE_CURRENT_SOURCE_DIR}/LedIndicator
    ${CMAKE_CURRENT_SOURCE_DIR}/Delegates
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core  # For NetworkInfo
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger
)

target_link_libraries(UtilitiesLibrary PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    CoreLibrary  # Link to Core
)
