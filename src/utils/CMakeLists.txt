#qt_add_executable(utils MapWidget.cpp jsonDataInterpreter.cpp)

set(RESOURCES ${RESOURCES}
    Data/WorldMap.png
    PARENT_SCOPE)

set(EXECUTABLES
    ${EXECUTABLES}
    ${CMAKE_CURRENT_LIST_DIR}/MapWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GitActions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/jsonFormatUtils.cpp
    PARENT_SCOPE)
#target_link_libraries(utils PRIVATE Qt6::Widgets)

#set_target_properties(utils PROPERTIES
#    WIN32_EXECUTABLE ON
#    MACOSX_BUNDLE ON
#)
