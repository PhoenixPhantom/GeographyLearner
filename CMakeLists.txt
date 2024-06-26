cmake_minimum_required(VERSION 3.16)

project(GeographyLearner VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)

#add_compile_options(-D EDITDATA="../../Data/configData.json")
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
qt_standard_project_setup()

if(MK_ED)
    add_compile_options(-DMAKE_EDITOR=1)
endif()

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/GeographyLearner.rc")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/libs/libgit2/win64/lib/)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/libs/libgit2/win64/bin/git2.dll ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/libgit2/include)
    #include_directories(${CMAKE_CURRENT_LIST_DIR}/libs/)
elseif(APPLE)
    message(WARNING "This CMake script has not yet been configured to work for apple devices \n
            If you have libgit2 installed and both direct access to the library files and the 
            header files are given in your PATH this warning can be ignored. \n Alternatively the paths
            can also be provided using -DCMAKE_PREFIX_PATH=path/to/your/library/and/header/files")
elseif(UNIX)
    message(WARNING "This CMake script has not yet been configured to work for linux \n
            If you have libgit2 installed and both direct access to the library files and the 
            header files are given in your PATH this warning can be ignored. \n Alternatively the paths
            can also be provided using -DCMAKE_PREFIX_PATH=path/to/your/library/and/header/files")
else()
    message(FATAL_ERROR "CMake cannot adequatly determine target platform")
endif()

find_library(LibGit2 git2 REQUIRED)

add_subdirectory(src)

add_executable(GeographyLearner ${EXECUTABLES} ${APP_ICON_RESOURCE_WINDOWS})

qt_add_resources(GeographyLearner "resources"
    PREFIX "/"
    FILES
    Data/Globe.ico
    ${RESOURCES}
)

qt_finalize_target(GeographyLearner)

target_link_libraries(GeographyLearner PUBLIC
    Qt6::Widgets
    ${LibGit2}
)

set_target_properties(GeographyLearner PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
