set(SOURCES local_control.c)

idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS .
                        REQUIRES freertos 
                        esp_common 
                        esp_event
                        nvs_flash
                        movement_control
                        position_control
                )