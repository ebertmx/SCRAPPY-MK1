// INCLUDE LIBRARIES
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "esp_log.h"

// INCLUDE HEADERS


static const char *PCTAG = "Position_Controller (PC)";


#define INCLUDE_vTaskSuspend 1
//FUNCTIONS

//TYPES

typedef struct VectorXYZ_t{
    int16_t x;
    int16_t y;
    int16_t z;
}VectorXYZ_t;

typedef struct VectorABC_t{
    int16_t a;
    int16_t b;
    int16_t c;
}VectorABC_t;

//MAIN TASK
void SCRP_PositionControl(void* args);

//convert from cartiesian end effector location to motor positions
esp_err_t xLocation2Position(VectorXYZ_t *xyz_loc, VectorABC_t *abc_pos);

//transmit the motor position to MovementControl task
esp_err_t xSendPosition (VectorABC_t *abc_pos);