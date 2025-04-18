cmake_minimum_required(VERSION 3.16)
project(ugnsm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Add subdirectories
add_subdirectory(Core)
add_subdirectory(UI)
add_subdirectory(Utilities)
add_subdirectory(Resources)

set(MAIN_SOURCES
    main.cpp
    mainwindow.cpp
)

# Main executable
qt_add_executable(ugnsm
    ${MAIN_SOURCES}
    $<TARGET_OBJECTS:ResourceLibrary>
)

target_link_libraries(ugnsm PRIVATE
    CoreLibrary
    UILibrary
    UtilitiesLibrary
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
)

target_include_directories(ugnsm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
install(TARGETS ugnsm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)
