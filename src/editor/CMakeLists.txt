#qt_add_executable(editor EditorWidget.cpp)

set(EXECUTABLES ${EXECUTABLES} ${CMAKE_CURRENT_LIST_DIR}/EditorWidget.cpp PARENT_SCOPE)
#target_link_libraries(editor PRIVATE Qt6::Widgets)

#set_target_properties(editor PROPERTIES
#    WIN32_EXECUTABLE ON
#    MACOSX_BUNDLE ON
#)
