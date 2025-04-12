cmake_minimum_required(VERSION 3.16)

project(ugnsm VERSION 0.1 LANGUAGES CXX)

# Enable automatic handling of .ui, .moc, and .qrc files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
)

# Create the executable (desktop only)
qt_add_executable(ugnsm ${PROJECT_SOURCES})

target_include_directories(ugnsm PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridCellWidgets"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridManager"
    "${CMAKE_CURRENT_SOURCE_DIR}/NetworkInformation"
)

target_link_libraries(ugnsm PRIVATE Qt6::Widgets Qt6::Network)

install(TARGETS ugnsm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
