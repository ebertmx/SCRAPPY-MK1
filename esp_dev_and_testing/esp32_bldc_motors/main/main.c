/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

// TaskHandle_t sensorA_isr_H = NULL;

pwm_motor motorA = {

    .pin_mcpwm = 16,
    .pin_dir = 17,
    .duty = 0,
    .dir = 0,
    .io = MCPWM0A,
    .unit = MCPWM_UNIT_0,
    .timer = MCPWM_TIMER_0,
    .gen = MCPWM_OPR_A};

pcnt_sensor sensorA = {
    .motor_pos = 0,
    .pin_pulse = 4,
    .pos_h_limit = 1000,
    .pos_l_limit = -1000,
    .count_dir = PCNT_COUNT_DEC,
    .unit = PCNT_UNIT_0,
    .channel = PCNT_CHANNEL_0};

pwm_motor motorB = {
    .pin_mcpwm = 19,
    .pin_dir = 21,
    .duty = 0,
    .dir = 0,
    .io = MCPWM1A,
    .unit = MCPWM_UNIT_0,
    .timer = MCPWM_TIMER_1,
    .gen = MCPWM_OPR_A};

pcnt_sensor sensorB = {
    .motor_pos = 0,
    .pin_pulse = 18,
    .pos_h_limit = 500,
    .pos_l_limit = -500,
    .count_dir = PCNT_COUNT_DEC,
    .unit = PCNT_UNIT_1,
    .channel = PCNT_CHANNEL_0};

pwm_motor motorC = {

    .pin_mcpwm = 26,
    .pin_dir = 27,
    .duty = 15,
    .dir = 0,
    .io = MCPWM2A,
    .unit = MCPWM_UNIT_0,
    .timer = MCPWM_TIMER_2,
    .gen = MCPWM_OPR_A};

pcnt_sensor sensorC = {
    .motor_pos = 0,
    .pin_pulse = 25,
    .pos_h_limit = 200,
    .pos_l_limit = -200,
    .count_dir = PCNT_COUNT_DEC,
    .unit = PCNT_UNIT_2,
    .channel = PCNT_CHANNEL_0};

void RunMotorA(void *p)
{
    const char *TAG = "MOTOR A";
    int16_t count_prev = 0;
    printf("%c", *TAG);

    int16_t count = 0;

    while (1)
    {
        pcnt_get_counter_value(sensorA.unit, &count);
        // vTaskDelay(10 / portTICK_RATE_MS);
        if (count < (sensorA.pos_l_limit + 100))
        {
            // printf("low limit = %d", (sensorA.pos_l_limit + 100));
            if (count != count_prev)
            {
                ESP_LOGI(TAG, "L_LIM EVT %d", count);
                setmotordir(&motorA, &sensorA, 1);
            }
        }

        if (count > (sensorA.pos_h_limit - 100))
        {
            if (count != count_prev)
            {
                ESP_LOGI(TAG, "H_LIM EVT %d", count);
                setmotordir(&motorA, &sensorA, 0);
            }
        }

        if (count != count_prev)
        {
          //  ESP_LOGI(TAG, "Current counter value :%d", count);
            count_prev = count;
        }
    }
}

void RunMotorB(void *p)
{
    int16_t count = 0;
    int16_t count_prev = 0;
    const char *TAG = "MOTOR B";
    printf("%c", *TAG);
    while (1)
    {
        pcnt_get_counter_value(sensorB.unit, &count);
        // vTaskDelay(10 / portTICK_RATE_MS);
        if (count < (sensorB.pos_l_limit + 100))
        {
            // printf("low limit = %d\n", (sensorA.pos_l_limit + 100));
            if (count != count_prev)
            {
                ESP_LOGI(TAG, "L_LIM EVT %d", count);
            }
            setmotordir(&motorB, &sensorB, 1);
        }
        if (count > (sensorB.pos_h_limit - 100))
        {
            if (count != count_prev)
            {
                ESP_LOGI(TAG, "H_LIM EVT %d", count);
            }
            setmotordir(&motorB, &sensorB, 0);
        }

        if (count != count_prev)
        {
          //  ESP_LOGI(TAG, "Current counter value :%d", count);
            count_prev = count;
        }
    }
}

void RunMotorC(void *p)
{
    int16_t count = 0;
    int16_t count_prev = 0;
    const char *TAG = "MOTOR C";
    printf("%c", *TAG);
    while (1)
    {
        pcnt_get_counter_value(sensorC.unit, &count);
        // vTaskDelay(10 / portTICK_RATE_MS);
        if (count < (sensorC.pos_l_limit + 100))
        {
            // printf("low limit = %d\n", (sensorA.pos_l_limit + 100));
            if (count != count_prev)
            {
                ESP_LOGI(TAG, "L_LIM EVT %d", count);
            }
            setmotordir(&motorC, &sensorC, 1);
        }
        if (count > (sensorC.pos_h_limit - 100))
        {
            if (count != count_prev)
            {
                ESP_LOGI(TAG, "H_LIM EVT %d", count);
            }
            setmotordir(&motorC, &sensorC, 0);
        }

        if (count != count_prev)
        {

            ESP_LOGI(TAG, "Current counter value :%d", count);
            count_prev = count;
        }
    }
}

TaskHandle_t RunAHandle = NULL;
TaskHandle_t RunBHandle = NULL;
TaskHandle_t RunCHandle = NULL;

void app_main(void)
{


    // pcnt_isr_service_install(0);
    ESP_ERROR_CHECK(bldc_init(&motorA, &sensorA));
    ESP_ERROR_CHECK(setmotorspeed(&motorA, 100));
    ESP_ERROR_CHECK(bldc_init(&motorB, &sensorB));
    ESP_ERROR_CHECK(setmotorspeed(&motorB, 50));
    ESP_ERROR_CHECK(bldc_init(&motorC, &sensorC));
    ESP_ERROR_CHECK(setmotorspeed(&motorC, 20));

    xTaskCreate(RunMotorA, "RunMotorA", 2000, (void *)1, tskIDLE_PRIORITY, &RunAHandle);
    xTaskCreate(RunMotorB, "RunMotorB", 2000, (void *)1, tskIDLE_PRIORITY, &RunBHandle);
   xTaskCreate(RunMotorC, "RunMotorC", 2000, (void *)1, tskIDLE_PRIORITY, &RunCHandle);
    // vTaskStartScheduler ();
    // vTaskDelete(&RunAHandle);
    // pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
    //  int16_t count = 0;
    //  pcnt_evt_t evt;
    //  portBASE_TYPE res;

    // ESP_ERROR_CHECK(bldc_init(&motorA, &sensorA));
    // ESP_ERROR_CHECK(setmotorspeed(&motorA, 25));

    // ESP_ERROR_CHECK(bldc_init(&motorB, &sensorB));
    // ESP_ERROR_CHECK(setmotorspeed(&motorB, 25));
    // ESP_ERROR_CHECK(bldc_init(&motorC, &sensorC));
    // ESP_ERROR_CHECK(setmotorspeed(&motorC, 25));

    // while (1)
    // {
    //     res = xQueueReceive(pcnt_evt_queue, &evt, 10 / portTICK_PERIOD_MS);
    //     if (res == pdTRUE)
    //     {
    //         pcnt_get_counter_value(sensorA.unit, &count);
    //         ESP_LOGI(TAG, "Event PCNT unit[%d]; cnt: %d", evt.unit, count);

    //         if (evt.status & PCNT_EVT_L_LIM)
    //         {
    //             ESP_LOGI(TAG, "L_LIM EVT");
    //             setmotordir(&motorA, &sensorA, 0);
    //             setmotordir(&motorB, &sensorB, 0);
    //             setmotordir(&motorC, &sensorC, 0);
    //         }
    //         if (evt.status & PCNT_EVT_H_LIM)
    //         {

    //             ESP_LOGI(TAG, "H_LIM EVT");

    //             setmotordir(&motorA, &sensorA, 1);
    //             setmotordir(&motorB, &sensorB, 1);
    //             setmotordir(&motorC, &sensorC, 1);
    //         }
    //     }
    //     else
    //     {
    //         pcnt_get_counter_value(sensorA.unit, &count);
    //         ESP_LOGI(TAG, "Current counter value :%d", count);
    //     }
    // }
}

// int16_t count = 0;
// pcnt_evt_t evt;
// portBASE_TYPE res;

// while (1)
// {

//     /* Wait for the event information passed from PCNT's interrupt handler.
//      * Once received, decode the event type and print it on the serial monitor.
//      */
//     res = xQueueReceive(pcnt_evt_queue, &evt, 500 / portTICK_PERIOD_MS);
//     if (res == pdTRUE)
//     {
//         pcnt_get_counter_value(motorA.unit, &count);
//         ESP_LOGI(TAG, "Event PCNT unit[%d]; cnt: %d", evt.unit, count);

//         if (evt.status & PCNT_EVT_L_LIM)
//         {
//             ESP_LOGI(TAG, "L_LIM EVT");
//             ESP_ERROR_CHECK(setmotordir(&motorA, 0));
//             ESP_ERROR_CHECK(setmotorspeed(&motorA, 50));
//             printf("motor dir = %d\n", motorA.dir);
//             gpio_set_level(2, 0);

//         }
//         if (evt.status & PCNT_EVT_H_LIM)
//         {
//             ESP_LOGI(TAG, "H_LIM EVT");
//             ESP_ERROR_CHECK(setmotordir(&motorA, 1));
//             ESP_ERROR_CHECK(setmotorspeed(&motorA, 25));
//             printf("motor dir = %d\n", motorA.dir);
//         }
//     }
//     else
//     {
//         pcnt_get_counter_value(motorA.unit, &count);
//         ESP_LOGI(TAG, "Current counter value :%d", count);
//     }

// }