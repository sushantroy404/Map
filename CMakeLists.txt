cmake_minimum_required(VERSION 3.16)

project(PathfindingVisualizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure Qt 6
find_package(Qt6 REQUIRED COMPONENTS Widgets Xml Gui Core)
qt_standard_project_setup()

add_executable(PathfindingVisualizer
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/Graph.h
    src/Graph.cpp
    src/OsmLoader.h
    src/OsmLoader.cpp
    src/CoordinateTransformer.h
    src/Algorithms.h
    src/Algorithms.cpp
    src/AnimationController.h
    src/AnimationController.cpp
    src/NodeItem.h
    src/NodeItem.cpp
    src/EdgeItem.h
    src/EdgeItem.cpp
)

target_link_libraries(PathfindingVisualizer PRIVATE
    Qt6::Widgets
    Qt6::Xml
    Qt6::Gui
    Qt6::Core
)

# Set runtime properties
if(WIN32)
    set_target_properties(PathfindingVisualizer PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()
