cmake_minimum_required(VERSION 3.16)

project(LineCounterProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find and link Qt6
find_package(Qt6 6.2 COMPONENTS
    Core
    Gui
    Widgets
    REQUIRED
)

add_executable(LineCounter
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(LineCounter
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
