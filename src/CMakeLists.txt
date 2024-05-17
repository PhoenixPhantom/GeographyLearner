#general project
add_subdirectory(utils)

if(MK_ED)
    add_subdirectory(editor)
    #set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -DMAKE_EDITOR=1 PARENT_SCOPE)
    add_compile_options(-DMAKE_EDITOR=1)
endif()

add_subdirectory(app)
set(RESOURCES ${RESOURCESS} Data/configData.json PARENT_SCOPE)

set(COMPILE_OPTIONS ${COMPILE_OPTIONS} PARENT_SCOPE)

set(RESOURCES ${RESOURCES}
    Data/Plus.ico
    Data/Minus.ico
    Data/Redo.ico
    Data/Right.ico
    Data/Left.ico
    Data/Upload.ico
    PARENT_SCOPE)

set(EXECUTABLES ${EXECUTABLES}
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProjectSelectorWidget.cpp
    PARENT_SCOPE)
#qt_add_executable(GeographyLearner main.cpp ${EXECUTABLES})

#target_link_libraries(GeographyLearner PRIVATE Qt6::Widgets)

#set_target_properties(GeographyLearner PROPERTIES
#    WIN32_EXECUTABLE ON
#    MACOSX_BUNDLE ON
#)
