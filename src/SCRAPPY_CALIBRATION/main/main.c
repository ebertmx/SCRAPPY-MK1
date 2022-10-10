/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

// GLOBALS
extern bool calibration = false;

// Task Handles and Queues
TaskHandle_t Handle_MovementControl = NULL;
extern QueueHandle_t xMC_queue;

QueueHandle_t xLC_queue;

// INTERRUPTS
static void IRAM_ATTR button_handler_isr(void *arg)
{
  uint32_t pin = (uint32_t)arg;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  // uint16_t debounce = 0x00;
  // for(int i = 0; i<16; i++){
  //     debouce = debounce | gpio_get_level(pin);
  // }
  xQueueSendFromISR(xLC_queue, &pin, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

esp_err_t gpio_init(void)
{
  // set up pins
  ESP_LOGI(LCTAG, "Starting Local Controller");

  gpio_config_t button = {
      .intr_type = GPIO_PIN_INTR_NEGEDGE,
      .pin_bit_mask = GPIO_BIT_MASK,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = 1,
      .pull_down_en = 0};
  gpio_config(&button);
  // gpio_set_intr_type(pin1, GPIO_INTR_ANYEDGE);
  // gpio_set_intr_type(pin2, GPIO_INTR_ANYEDGE);
  // gpio_set_intr_type(pin3, GPIO_INTR_ANYEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(pin1, button_handler_isr, (void *)pin1);
  // gpio_isr_handler_add(pin2, button_handler_isr, (void *)pin2);
  gpio_isr_handler_add(pin3, button_handler_isr, (void *)pin3);
}





void app_main(void)
{
  ESP_LOGI(MainTAG, "Initiating Startup: SCRAPPY");
  // SET UP QUEUES
  xMC_queue = xQueueCreate(10, sizeof(int16_t[6]));
  // xPC_queue = xQueueCreate(10, sizeof(int16_t));
  // xLC_queue = xQueueCreate(10, sizeof(uint32_t));

  // CREATE TASKS
  ESP_LOGI(MainTAG, "Starting Tasks");
  xTaskCreatePinnedToCore(SCRP_MovementControl, "MC", 4096, NULL, 10, &Handle_MovementControl, 1);

  //
  ESP_LOGI(MainTAG, "Calibrating");
  calibration = true;
  // vTaskResume(Handle_PositionControl);
  ESP_LOGI(MainTAG, "Calibration Success");

  int16_t myposition1[] = {-100, -100, 0, 40, 40, 40};

  int16_t myposition2[] = {100, 100, 0, 60, 60, 60};
  while (1)
  {
    xQueueSendToBack(xMC_queue, (void *)&(myposition2), portMAX_DELAY);
    vTaskDelay(4000 / portTICK_RATE_MS);
    //  myposition1[1]-= 100;

    xQueueSendToBack(xMC_queue, (void *)&(myposition1), portMAX_DELAY);
    vTaskDelay(4000 / portTICK_RATE_MS);
    //  ESP_LOGI(MCTAG, "ASHTON, I sent this...");
    //  myposition2[1]+= 100;
  }
}
