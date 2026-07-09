project(gstcviivebackground)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/gstcviivebackground.c)

add_library(${PROJECT_NAME} SHARED ${SRC})
target_link_libraries(${PROJECT_NAME} ${PLUGIN_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION gst-plugin)