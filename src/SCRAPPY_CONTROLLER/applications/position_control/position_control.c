
/*  Created: 2022-10-02 By: ebertmx

*Description: This task calculates and send encoder position to SCRAPPY's movement controller.
    This task deals with XYZ coordinates and the environment. To use this task properly SCRAPPY
    must be in a calibrated state
*A list of function and components can be found in position_control.h
*/
#include "position_control.h"

// GLOBAL
QueueHandle_t xPC_queue;
//bool calibration = false;

extern QueueHandle_t xMC_queue;
int16_t txbuff[10];
int16_t Z;

VectorABC_t SCRP_target_position;
VectorXYZ_t SCRP_target_location;

// INTERRUPTS

/**************MAIN TASK *********************/

void SCRP_PositionControl(void *args)
{
    ESP_LOGI(PCTAG, "Initiating Position Control");
    // suspend task until calibrated and woken
    while (false == calibration)
    {
        ESP_LOGI(PCTAG, "Please Calibrate SCRAPPY...");
        vTaskSuspend(NULL);
    }

    while (1)
    {
        if (xQueueReceive(xPC_queue, &SCRP_target_location, portMAX_DELAY))
        {
            ESP_LOGI(PCTAG, "Recieved a target Location");
            ESP_ERROR_CHECK(xLocation2Position(&SCRP_target_location, &SCRP_target_position));
            ESP_ERROR_CHECK(xSendPosition(&SCRP_target_position));
            ESP_LOGI(PCTAG, "Sent a target Position");
        }
    }
}

/*******************************************/

// FUNCTION
esp_err_t xLocation2Position(VectorXYZ_t *xyz_loc, VectorABC_t *abc_pos)
{
    abc_pos->a = xyz_loc->x;
    abc_pos->b = xyz_loc->y;
    abc_pos->c = xyz_loc->z;
    return ESP_OK;
}

esp_err_t xSendPosition(VectorABC_t *abc_pos)
{
    xQueueSend(xMC_queue, (void *)(abc_pos), (TickType_t)0);
    return ESP_OK;
}