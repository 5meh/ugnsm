file(GLOB_RECURSE UTILS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Parsers/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Parsers/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/LedIndicator/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LedIndicator/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Delegates/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Delegates/*.h"
)

add_library(UtilitiesLibrary STATIC ${UTILS_SOURCES})

target_include_directories(UtilitiesLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Parsers
    ${CMAKE_CURRENT_SOURCE_DIR}/LedIndicator
    ${CMAKE_CURRENT_SOURCE_DIR}/Delegates
)

target_link_libraries(UtilitiesLibrary PUBLIC Qt6::Core Qt6::Widgets Qt6::Gui)
