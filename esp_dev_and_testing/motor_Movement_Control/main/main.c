/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

#define GPIO_BIT_MASK ((1ULL << GPIO_NUM_22))
#define buttonpin 22

// Task Handles
TaskHandle_t Handle_MovementControl = NULL;
extern QueueHandle_t xMC_queue;


TaskHandle_t Handle_SendPosition = NULL;
QueueHandle_t xLC_queue;
int16_t txbuff[10];
// static xQueueHandle gpio_evt_queue = NULL;

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

void xSendPosition(void *arg)
{
    txbuff[0] = 100;
    txbuff[1] = -100;
    txbuff[2] = 1000;

    uint32_t pin;
    if (xQueueReceive(xLC_queue, &pin, portMAX_DELAY))
    {
        for (int i = 0; i < 3; i = i + 1)
        {
            //sprintf(txbuff, "hello world! 1");
            xQueueSend(xMC_queue, (void *)&(txbuff[i]), (TickType_t)0);
        }
        while (1)
        {

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(MainTAG, "Initiating Startup: SCRAPPY");
    // SET UP QUEUES
    xMC_queue = xQueueCreate(5, sizeof(txbuff));
    xLC_queue = xQueueCreate(10, sizeof(int32_t));
    // if (NULL == xPosSendQueue)
    // {
    //     ESP_LOGI(MainTAG, "Failed to create message QUEUE for PosSend");
    // }

    // // GPIO
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
    xTaskCreatePinnedToCore(SCRP_MovementControl, "MC", 4096, &xMC_queue, 10, &Handle_MovementControl, 1);
    xTaskCreate(xSendPosition, "Sender", 4096, NULL, 10, &Handle_SendPosition);
}