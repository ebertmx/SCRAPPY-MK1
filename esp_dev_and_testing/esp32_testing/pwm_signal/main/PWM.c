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
static const char *TAG = "PWM";

#define MOTOR_PIN (5)

void app_main(void)
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_PIN);

    mcpwm_config_t pwm_config_motor1 = {
        .frequency = 50,
        .cmpr_a = 0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config_motor1);


    // for example 1000 us of duty
    int cnt = 0;
    while (1)
    {
        printf("cnt= %d\n", cnt);
        cnt++;
        ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, cnt % 1000));
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}