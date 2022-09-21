/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
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

#include "esp_attr.h"
#include "esp_log.h"

static const char *TAG = "PWM";

#define PCNT_H_LIM_VAL 1000
#define PCNT_L_LIM_VAL -1000
#define PCNT_THRESH1_VAL 5
#define PCNT_THRESH0_VAL 0
#define PCNT_INPUT_SIG_IO 19  // Pulse Input GPIO
#define PCNT_INPUT_CTRL_IO 23 // Control GPIO HIGH=count up, LOW=count down

xQueueHandle pcnt_evt_queue; // A queue to handle pulse counter events

/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 */
typedef struct
{
    int unit;        // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
{
    int pcnt_unit = (int)arg;
    pcnt_evt_t evt;
    evt.unit = pcnt_unit;
    /* Save the PCNT event type that caused an interrupt
       to pass it to the main program */
    pcnt_get_event_status(pcnt_unit, &evt.status);
    xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
}

static void pcnt_example_init(int unit)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC, // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS, // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(unit, 100);
    pcnt_filter_enable(unit);

    /* Set threshold 0 and 1 values and enable events to watch */
    pcnt_set_event_value(unit, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    pcnt_event_enable(unit, PCNT_EVT_THRES_1);
    pcnt_set_event_value(unit, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    pcnt_event_enable(unit, PCNT_EVT_THRES_0);
    /* Enable events on zero, maximum and minimum limit values */
    // pcnt_event_enable(unit, PCNT_EVT_ZERO);
    pcnt_event_enable(unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(unit, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);

    /* Install interrupt service and add isr callback handler */
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(unit, pcnt_example_intr_handler, (void *)unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(unit);
}

#define MOTOR_PIN (5)

void app_main(void)
{

    int pcnt_unit = PCNT_UNIT_0;
    pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
    pcnt_example_init(pcnt_unit);

    int16_t count = 0;
    pcnt_evt_t evt;
    portBASE_TYPE res;

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_PIN);
    mcpwm_config_t pwm_config_motor1 = {
        .frequency = 15000,
        .cmpr_a = 1,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_1,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config_motor1);

    gpio_set_direction(GPIO_NUM_22, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_22, 0);

    ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50));
int speed = 50;
int counter = 0;
    while (1)
    {
       
        /* Wait for the event information passed from PCNT's interrupt handler.
         * Once received, decode the event type and print it on the serial monitor.
         */
        res = xQueueReceive(pcnt_evt_queue, &evt, 1000 / portTICK_PERIOD_MS);
        if (res == pdTRUE)
        {
            pcnt_get_counter_value(pcnt_unit, &count);
            ESP_LOGI(TAG, "Event PCNT unit[%d]; cnt: %d", evt.unit, count);
            if (evt.status & PCNT_EVT_THRES_1)
            {
                ESP_LOGI(TAG, "THRES1 EVT");
            }
            if (evt.status & PCNT_EVT_THRES_0)
            {
                ESP_LOGI(TAG, "THRES0 EVT");
            }
            if (evt.status & PCNT_EVT_L_LIM)
            {
                ESP_LOGI(TAG, "L_LIM EVT");
                vTaskDelay(3000 / portTICK_PERIOD_MS);
            }
            if (evt.status & PCNT_EVT_H_LIM)
            {
                 counter++;
                ESP_LOGI(TAG, "H_LIM EVT");
                ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0));
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, (speed+50*(counter%2))));
            }
            if (evt.status & PCNT_EVT_ZERO)
            {
                ESP_LOGI(TAG, "ZERO EVT");
            }
        }
        else
        {
            pcnt_get_counter_value(pcnt_unit, &count);
            ESP_LOGI(TAG, "Current counter value :%d", count);
        }
    }
}