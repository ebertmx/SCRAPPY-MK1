#include "bldc.h"

static void IRAM_ATTR sensor_isr_handler(void *arg)
{
    int pcnt_unit = (int)arg;
    pcnt_evt_t evt;
    evt.unit = pcnt_unit;
    /* Save the PCNT event type that caused an interrupt
       to pass it to the main program */
    pcnt_get_event_status(pcnt_unit, &evt.status);
    // printf("Unit: %d    Event: %c", evt.unit, evt.status);
    // xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
}

esp_err_t bldc_init(struct pwm_motor *motor, struct pcnt_sensor *sensor)
{

    // MOTOR

    ESP_ERROR_CHECK(mcpwm_gpio_init(motor->unit, motor->io, motor->pin_mcpwm)); // pwm pin

    mcpwm_config_t pwm_config_motor1 = {
        .frequency = 20000,
        .cmpr_a = motor->duty,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };

    mcpwm_init(motor->unit, motor->timer, &pwm_config_motor1);

    gpio_set_direction(motor->pin_dir, GPIO_MODE_OUTPUT);

    //gpio_set_level(motor->pin_dir, motor->dir);

    // SENSOR
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = sensor->pin_pulse,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .channel = sensor->channel, // PCNT_CHANNEL_0,
        .unit = sensor->unit,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = sensor->count_dir, // Count on the positive edge
        .neg_mode = PCNT_COUNT_DIS,    // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP, // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = sensor->pos_h_limit,
        .counter_l_lim = sensor->pos_l_limit,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(sensor->unit, 100);
    pcnt_filter_enable(sensor->unit);

    pcnt_event_enable(sensor->unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(sensor->unit, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(sensor->unit);
    pcnt_counter_clear(sensor->unit);

    /* Install interrupt service and add isr callback handler */

    pcnt_isr_service_install(0);

    pcnt_isr_handler_add(sensor->unit, sensor_isr_handler, (void *)sensor->unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(sensor->unit);


//set up direction and speed
   ESP_ERROR_CHECK( setmotordir(motor,sensor,motor->dir));
    ESP_ERROR_CHECK(setmotorspeed(motor,motor->duty));
    return ESP_OK;
}

esp_err_t setmotordir(struct pwm_motor *motor, struct pcnt_sensor *sensor, uint16_t direction)
{
    motor->dir = direction;
    gpio_set_level(motor->pin_dir, motor->dir);
    if (1 == motor->dir)
    {
        sensor->count_dir = PCNT_COUNT_INC;
    }
    if (0 == motor->dir)
    {
        sensor->count_dir = PCNT_COUNT_DEC;
    }
   // printf("sensor direction %d\n", sensor->count_dir);
    pcnt_set_mode(sensor->unit, sensor->channel, sensor->count_dir, PCNT_COUNT_DIS, PCNT_MODE_KEEP, PCNT_MODE_KEEP);
    return ESP_OK;
}

esp_err_t setmotorspeed(struct pwm_motor *motor, float speed)
{
    motor->duty = speed;
    printf("motor speed= %f\n", motor->duty);

    ESP_ERROR_CHECK(mcpwm_set_duty(motor->unit, motor->timer, motor->gen, motor->duty));

    return ESP_OK;
}
