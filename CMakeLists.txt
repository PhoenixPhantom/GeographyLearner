cmake_minimum_required(VERSION 3.16)

project(GeographyLearner VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)

#add_compile_options(-D EDITDATA="../../Data/configData.json")
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
qt_standard_project_setup()

add_subdirectory(src)

#I know this is not nice to do here, but I can't figure out why passing the options in a macro doesn't work
if(MK_ED)
    add_compile_options(-DMAKE_EDITOR=1)
endif()

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/GeographyLearner.rc")
endif()

add_executable(GeographyLearner ${EXECUTABLES} ${APP_ICON_RESOURCE_WINDOWS})

qt_add_resources(GeographyLearner "resources"
    PREFIX "/"
    FILES
    Data/Globe.ico
    ${RESOURCES}
)

qt_finalize_target(GeographyLearner)

target_link_libraries(GeographyLearner PRIVATE Qt6::Widgets)

set_target_properties(GeographyLearner PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

