set(SOURCES bldc.c)

idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS .
                )