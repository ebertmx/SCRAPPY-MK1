set(SOURCES main.c)

idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES freertos 
                    esp_common 
                    esp_event
                    nvs_flash
                    movement_control
                    position_control
                    local_control
                  )
