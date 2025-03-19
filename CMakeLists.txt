cmake_minimum_required(VERSION 3.16)

project(ugnsm VERSION 0.1 LANGUAGES CXX)

# Enable automatic handling of .ui, .moc, and .qrc files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Widgets and Network modules
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# List your source files (add any additional ones as needed)
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    networkinfoviewwidget.h
    networkinfoviewwidget.cpp
    networkinfoview.h
    networkinfoview.cpp
    ledindicator.h
    ledindicator.cpp
)

# Create the executable (desktop only)
qt_add_executable(ugnsm ${PROJECT_SOURCES}
    ledindicatordelegate.h ledindicatordelegate.cpp
    networkinfo.h networkinfo.cpp
    networkinfoview.h networkinfoview.cpp)

target_link_libraries(ugnsm PRIVATE Qt6::Widgets Qt6::Network)

install(TARGETS ugnsm
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(ugnsm)
