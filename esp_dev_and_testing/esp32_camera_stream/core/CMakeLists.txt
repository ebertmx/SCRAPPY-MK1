set(SOURCES ./src/main.cpp)

idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS . include
                        REQUIRES freertos 
                                    esp_common 
                                    esp_event
                                    esp_http_server 
                                    nvs_flash
                                    Wifi
                                    esp32-camera-master
                                    camera1
                                    Server1)