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
#include "driver/gpio.h"
#include "driver/pcnt.h"
// static const char TAG = "BLDC";

#define PCNT_H_LIM_VAL 500
#define PCNT_L_LIM_VAL -500
#define PCNT_THRESH1_VAL 5
#define PCNT_THRESH0_VAL 0





typedef struct pwm_motor
{
    uint16_t pin_mcpwm;
    uint16_t pin_dir;

    float duty;
    int dir;

    mcpwm_io_signals_t io;
    mcpwm_unit_t unit;
    mcpwm_timer_t timer;
    mcpwm_generator_t gen;

} pwm_motor;

typedef struct pcnt_sensor
{
    int motor_pos;

    uint16_t pin_pulse;
    int16_t pos_h_limit;
    int16_t pos_l_limit;

    pcnt_count_mode_t count_dir;

    pcnt_unit_t unit;
    pcnt_channel_t channel;

} pcnt_sensor;





xQueueHandle pcnt_evt_queue;

typedef struct
{
    int unit;        // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

esp_err_t bldc_init(struct pwm_motor *motor, struct pcnt_sensor *sensor);
esp_err_t setmotordir(struct pwm_motor *motor, struct pcnt_sensor *sensor, uint16_t direction);
esp_err_t setmotorspeed(struct pwm_motor *motor, float speed);
