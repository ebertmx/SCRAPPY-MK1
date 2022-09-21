set(SOURCES camera1.cpp)

idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS .
                        REQUIRES esp32-camera-master esp_event)

# find_package( OpenCV REQUIRED )
# find_package( VISP REQUIRED )
# include_directories( ${OpenCV_INCLUDE_DIRS} "C:\\VISP_WS\\visp_build_mingw\\install\\include")
# target_link_libraries(${PROJECT_NAME} ${SOURCES} ${OpenCV_LIBS} ${VISP_LIBS})
