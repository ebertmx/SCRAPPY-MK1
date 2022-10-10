/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

// GLOBALS
extern bool calibration;

// Task Handles and Queues
TaskHandle_t Handle_MovementControl = NULL;
extern QueueHandle_t xMC_queue;

TaskHandle_t Handle_PositionControl = NULL;
extern QueueHandle_t xPC_queue;

TaskHandle_t Handle_LocalControl = NULL;
extern QueueHandle_t xLC_queue;

void app_main(void)
{
    ESP_LOGI(MainTAG, "Initiating Startup: SCRAPPY");
    // SET UP QUEUES
    xMC_queue = xQueueCreate(10, sizeof(int16_t[6]));
    xPC_queue = xQueueCreate(10, sizeof(int16_t));
    xLC_queue = xQueueCreate(10, sizeof(uint32_t));

    // CREATE TASKS
    ESP_LOGI(MainTAG, "Starting Tasks");
    xTaskCreatePinnedToCore(SCRP_MovementControl, "MC", 4096, NULL, 10, &Handle_MovementControl, 1);
    // xTaskCreatePinnedToCore(SCRP_PositionControl, "PC", 4096, NULL, 0, &Handle_PositionControl, 0);
    // xTaskCreatePinnedToCore(SCRP_LocalController, "LC", 2000, NULL, 9, &Handle_LocalControl,1);

    //
    ESP_LOGI(MainTAG, "Calibrating");
    calibration = true;
    //vTaskResume(Handle_PositionControl);
    ESP_LOGI(MainTAG, "Calibration Success");

    int16_t myposition1[] = {-100, -100, 0, 40, 40, 40};

   int16_t myposition2[] = {100, 100, 0, 60, 60, 60};
    while(1){
    xQueueSendToBack(xMC_queue, (void *)&(myposition2), portMAX_DELAY);
    vTaskDelay(4000 / portTICK_RATE_MS);
    //  myposition1[1]-= 100;

    xQueueSendToBack(xMC_queue, (void *)&(myposition1),  portMAX_DELAY);
       vTaskDelay(4000 / portTICK_RATE_MS);
     //  ESP_LOGI(MCTAG, "ASHTON, I sent this...");
   //  myposition2[1]+= 100;
    }
}
