#qt_add_executable(app LearnerWidget.cpp)

set(EXECUTABLES ${EXECUTABLES} ${CMAKE_CURRENT_LIST_DIR}/LearnerWidget.cpp PARENT_SCOPE)


set(RESOURCES ${RESOURCES}
    Data/Menu.ico
    PARENT_SCOPE)


#target_link_libraries(app PRIVATE Qt6::Widgets)

#set_target_properties(app PROPERTIES
#   WIN32_EXECUTABLE ON
#   MACOSX_BUNDLE ON
#)
