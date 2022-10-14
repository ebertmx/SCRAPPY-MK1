/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

// GLOBALS
extern TaskHandle_t xh_MC;
extern QueueHandle_t xMC_queue;

void app_main(void)
{
    esp_log_level_set("MC-MOTOR", ESP_LOG_NONE);
    ESP_LOGI(SCRP, "SCRAPPY Starting...");

    ESP_LOGI(SCRP, "Creating Tasks...");
    xTaskCreatePinnedToCore(SCRP_MovementControl, "MC", 4086, NULL, 5, &xh_MC, 1);
    xMC_queue = xQueueCreate(10, sizeof(int16_t[7]));
    int count = 0;

    ESP_LOGI(SCRP, "Standing by...");

    int16_t myposition1[] = {'P', -1000, -100, -100, 75, 30, 75};
    int16_t myposition2[] = {'P', 1000, 100, -100, 50, 30, 60};
    int i = 0;
    while (i < 5)
    {
        ESP_LOGI(SCRP, "SENDING...");
        xQueueSendToBack(xMC_queue, (void *)&(myposition2), portMAX_DELAY);
        myposition2[1] += 100;
        vTaskDelay(100 / portTICK_RATE_MS);
        ESP_LOGI(SCRP, "SENDING...");
        xQueueSendToBack(xMC_queue, (void *)&(myposition1), portMAX_DELAY);
        myposition1[1] += -100;
        vTaskDelay(10 / portTICK_RATE_MS);
        i++;
    }

    while (1)
    {
        vTaskDelay(3000 / portTICK_RATE_MS);
        ESP_LOGI(SCRP, "Standing by...");
    }
}
