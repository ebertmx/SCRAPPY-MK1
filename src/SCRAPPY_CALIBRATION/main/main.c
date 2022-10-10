/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

// GLOBALS
bool calibration = false;

// Task Handles and Queues
TaskHandle_t Handle_MovementControl = NULL;
extern QueueHandle_t xMC_queue;

// QueueHandle_t xLC_queue;

// INTERRUPTS
// static void IRAM_ATTR button_handler_isr(void *arg)
// {
//   uint32_t pin = (uint32_t)arg;

//   BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//   uint16_t debounce = 0xFFFF;
//   for (int i = 0; i < 32; i++)
//   {
//     debounce = (debounce << 1) | (gpio_get_level(pin) & 0xFFFF);
//   }
//   if (debounce == 0x00)
//   {
//     xQueueSendFromISR(xLC_queue, &pin, &xHigherPriorityTaskWoken);
//   }
//   if (xHigherPriorityTaskWoken)
//   {
//     portYIELD_FROM_ISR();
//   }
// }

esp_err_t gpio_initiate(void)
{
  // set up pins

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
  // gpio_install_isr_service(0);
  // gpio_isr_handler_add(pin1, button_handler_isr, (void *)pin1);
  // gpio_isr_handler_add(pin2, button_handler_isr, (void *)pin2);
  // gpio_isr_handler_add(pin3, button_handler_isr, (void *)pin3);

  return ESP_OK;
}

void app_main(void)
{
  ESP_LOGI(MainTAG, "Initiating Startup: SCRAPPY");
  // SET UP QUEUES
  xMC_queue = xQueueCreate(1, sizeof(int16_t[6]));
  // xPC_queue = xQueueCreate(10, sizeof(int16_t));
  // xLC_queue = xQueueCreate(1, sizeof(uint32_t));

  // CREATE TASKS
  ESP_LOGI(MainTAG, "Starting Tasks");
  xTaskCreatePinnedToCore(SCRP_MovementControl, "MC", 4096, NULL, 10, &Handle_MovementControl, 1);

  //

  gpio_initiate();
  ESP_LOGI(MainTAG, "Calibrating");
  calibration = true;
  // vTaskResume(Handle_PositionControl);
  ESP_LOGI(MainTAG, "Calibration Success");
  uint32_t pin;
  int count = 0;
  int16_t ticks;
  int16_t myposition1[] = {-10, 0, 0, 30, 0, 0};

  int16_t myposition2[] = {10, 0, 0, 30, 0, 0};
  int16_t myposition3[] = {0, -10, 0, 0, 30, 0};
  int16_t myposition4[] = {0, 10, 0, 0, 30, 0};
  ESP_LOGE(MainTAG, "Press Button When Ready");
  while (1)
  {

    if (!gpio_get_level(22))
    {
      ESP_LOGE(MainTAG, "Moving To Start");
      vTaskDelay(1000 / portTICK_RATE_MS);
      while (gpio_get_level(22))
      {
        myposition3[1] += -10;
        xQueueSendToBack(xMC_queue, (void *)&(myposition3), portMAX_DELAY);
      }
      ESP_LOGE(MainTAG, "Counting Ticks");
      count = 0;
      vTaskDelay(1000 / portTICK_RATE_MS);
      while (gpio_get_level(22))
      {
        myposition3[1] += 10;
        xQueueSendToBack(xMC_queue, (void *)&(myposition3), portMAX_DELAY);
        count = count + 1;
      }
      ticks = count * 10;
      ESP_LOGE(MainTAG, "TICKs COUNT = %d", ticks);
       vTaskDelay(2000 / portTICK_RATE_MS);
    }
count = count+1;
     vTaskDelay(1);
  }
}
