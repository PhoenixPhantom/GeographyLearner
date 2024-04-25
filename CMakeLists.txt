cmake_minimum_required(VERSION 3.16)

project(GeographyLearner VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
qt_standard_project_setup()

add_subdirectory(src/app)

qt_add_resources(GeographyLearner "resources"
    PREFIX "/"
    FILES
        Data/WorldMap.png
        Data/DotSelector.png
        Data/configData.json
)

#add_definitions(-DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DNDEBUG -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB)

target_link_libraries(GeographyLearner PRIVATE Qt6::Widgets)

set_target_properties(GeographyLearner PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

