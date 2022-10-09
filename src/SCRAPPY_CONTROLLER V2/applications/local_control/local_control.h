// INCLUDE LIBRARIES
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "esp_log.h"
#include "driver/gpio.h"

// INCLUDE HEADERS


static const char *LCTAG = "Local_Controller (LC)";


#define INCLUDE_vTaskSuspend 1
//#define GPIO_BIT_MASK ((1ULL << GPIO_NUM_15) | (1ULL << GPIO_NUM_2) |(1ULL << GPIO_NUM_1))
#define pin1 23
#define pin2 2
#define pin3 22

#define GPIO_BIT_MASK (1ULL << GPIO_NUM_23)|(1ULL<<GPIO_NUM_22) //| (1ULL<<GPIO_NUM_2)

//FUNCTIONS

void SCRP_LocalController(void *argv);

//TYPES
