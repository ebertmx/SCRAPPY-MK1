// INCLUDE LIBRARIES
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "esp_log.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"

// INCLUDE HEADERS

// CONSTANTS
#define motorHlimit 30000
#define motorLlimit -30000
#define PWMFREQUENCY 20000

static const char *MCTAG = "Movement_Controller (MC)";

// DATA TYPES

// For motor direction
typedef enum direction_t
{
    dir_pos = 0,
    dir_neg = 1
} direction_t;

typedef struct motor_pos_t
{
    char motor;        // the PCNT unit that originated an interrupt
    uint32_t enc_pos; // information on the event type that caused the interrupt
} motor_pos_t;


typedef struct xSCRP_motor_t
{
    // hardware pins
    uint16_t pin_pwm;
    uint16_t pin_dir;
    uint16_t pin_enc;

    // settable properties
    float speed;
    int direction;

    int16_t position;
    // readable properties
    int region;

    // mcpwm setting
    mcpwm_io_signals_t pwm_io;
    mcpwm_unit_t pwm_unit;
    mcpwm_timer_t pwm_timer;
    mcpwm_generator_t pwm_gen;

    // pcnt settings
    pcnt_count_mode_t enc_dir;
    pcnt_unit_t enc_unit;
    pcnt_channel_t enc_channel;

} xSCRP_motor_t;

typedef struct enc_evt_t
{
    int unit;        // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} enc_evt_t;

// Our main task function
void SCRP_MovementControl(void *args);

// initiated a motor and cooresponding counter
esp_err_t xMotorSetUp(struct xSCRP_motor_t *SCRP_motor);
// set the direction of the motor and counter
esp_err_t xSetMotorDir(struct xSCRP_motor_t *SCRP_motor, direction_t dir);
// set the motor running at a defined speed
esp_err_t xSetMotorSpeed(struct xSCRP_motor_t *SCRP_motor, float speed);
// immediately stop a motor
esp_err_t xStopMotor(struct xSCRP_motor_t *SCRP_motor);
// resume the motor running at the last set speed
esp_err_t xResumeMotor(struct xSCRP_motor_t *SCRP_motor);
// set a encoder position target; Update the absolute position before setting the new target
esp_err_t xSetTarget(struct xSCRP_motor_t *SCRP_motor, int16_t target);
// reset a encoder count
esp_err_t xOverideCurrentPosition(void *newposition);
// get the relative position of motor wrt the last absolute position
int16_t xGetEncoderValue(struct xSCRP_motor_t *SCRP_motor);
//get the current absolute position (last save position + encoder value) of motor
//Does not update absolute position
int16_t xGetPosition(struct xSCRP_motor_t *SCRP_motor);
//update current absolute position by adding the current encoder count; 
//WARNING clears the current target value;
esp_err_t xUpdatePosition(struct xSCRP_motor_t *SCRP_motor);
