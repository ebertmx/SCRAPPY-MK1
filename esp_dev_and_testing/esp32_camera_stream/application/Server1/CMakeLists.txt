set(SOURCES Server1.cpp)

idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS .
                        REQUIRES esp_http_server esp_http_client esp-tls esp32-camera-master camera1)

