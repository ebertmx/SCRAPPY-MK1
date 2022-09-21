set(SOURCES main.c)

idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES freertos 
                    esp_common 
                    esp_event
                    nvs_flash
                    bldc
                  )
