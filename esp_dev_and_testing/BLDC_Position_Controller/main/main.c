/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

#define GPIO_BIT_MASK ((1ULL << GPIO_NUM_22))
#define buttonpin 22

// GLOBALS
extern bool calibration;

// Task Handles and Queues
TaskHandle_t Handle_MovementControl = NULL;
extern QueueHandle_t xMC_queue;

TaskHandle_t Handle_PositionControl = NULL;
extern QueueHandle_t xPC_queue;

QueueHandle_t xLC_queue;

static void IRAM_ATTR button_handler_isr(void *arg)
{
    uint32_t pin = (uint32_t)arg;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(xLC_queue, &pin, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

void app_main(void)
{
    ESP_LOGI(MainTAG, "Initiating Startup: SCRAPPY");
    // SET UP QUEUES
    xMC_queue = xQueueCreate(10, sizeof(int16_t));
    xPC_queue = xQueueCreate(10, sizeof(VectorABC_t));
    xLC_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t button = {
        .intr_type = GPIO_PIN_INTR_POSEDGE,
        .pin_bit_mask = GPIO_BIT_MASK,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0};
    gpio_config(&button);
    gpio_set_intr_type(buttonpin, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(buttonpin, button_handler_isr, (void *)buttonpin);

    // CREATE TASKS
    ESP_LOGI(MainTAG, "Starting Tasks");
    xTaskCreatePinnedToCore(SCRP_MovementControl, "MC", 4096, NULL, 10, &Handle_MovementControl, 1);
    xTaskCreatePinnedToCore(SCRP_PositionControl, "PC", 4096, NULL, 5, &Handle_PositionControl, 0);


    //
    ESP_LOGI(MainTAG, "Calibrating");
    calibration =true;
    vTaskResume  (Handle_PositionControl);
    ESP_LOGI(MainTAG, "Calibration Success");

    VectorXYZ_t myposition = {1000, 2000, 3000};
    ESP_LOGI(MainTAG, "Waiting for Input");
    uint32_t pin;
    if (xQueueReceive(xLC_queue, &pin, portMAX_DELAY))

    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        ESP_LOGI(MainTAG, "Sending Location 1");
        xQueueSend(xPC_queue, (void *)&(myposition), 0);
        myposition.x = -1000;
        myposition.y = -2000;
        myposition.z = -3000;
        vTaskDelay(3000 / portTICK_RATE_MS);
        ESP_LOGI(MainTAG, "Sending Location 2");
        xQueueSend(xPC_queue, (void *)&(myposition), 0);
    }
}