add_subdirectory(app)
add_subdirectory(utils)
add_subdirectory(editor)

qt_add_executable(GeographyLearner main.cpp ${EXECUTABLES})

#target_link_libraries(GeographyLearner PRIVATE Qt6::Widgets)

#set_target_properties(GeographyLearner PROPERTIES
#    WIN32_EXECUTABLE ON
#    MACOSX_BUNDLE ON
#)
