#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"

#include "esp_log.h"

static const char *MainTAG = "MainProgram";

//#include "local_control.h"
#include "movement_control.h"
//#include "position_control.h"

#define INCLUDE_vTaskSuspend 1



#define pin1 23
#define pin2 2
#define pin3 22
#define GPIO_BIT_MASK (1ULL<<GPIO_NUM_22)//(1ULL << GPIO_NUM_23)|
