
// INCLUDE LIBRARIES
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "soc/rtc.h"
#include "soc/rtc.h"
#include "esp_log.h"
#include "string.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"

/****************TAGS*****************/
static const char *MC = "MC";
static const char *MCSTOP = "MC-STOP";
static const char *MCRUN = "MC-RUN";
static const char *MCSET = "MC-SET";
static const char *MCMOTORS = "MC-MOTOR";

/**********MOTOR CONSTANTS************/
#define motorHlimit 30000
#define motorLlimit -30000
#define PWMFREQUENCY 20000

#define motorpwm0 19
#define motordir0 21
#define motorenc0 18
#define motorpwm1 26
#define motordir1 27
#define motorenc1 25
#define motorpwm2 16
#define motordir2 17
#define motorenc2 4


#define DUTYMODE MCPWM_DUTY_MODE_0

#define MOTOR2_CORR_FACTOR 2
/*****************DATA TYPES*************/

// For motor direction
typedef enum direction_t
{
    DIRPOS = 0,
    DIRNEG = 1
} direction_t;

typedef struct xSCRP_motor_t
{
    int16_t num;
    // hardware pins
    uint16_t pin_pwm;
    uint16_t pin_dir;
    uint16_t pin_enc;

    // settable properties
    uint16_t speed;
    uint16_t minspeed;
    float Kp;
    float signal;
    int16_t error;
    int16_t errorprev;
    unsigned int count;
    int direction;
    int16_t position;
    int16_t target;
    int16_t enc_target;
    bool enable;

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

// void MC_Set(void *args);
void MC_Run(void *args);
void MC_Stop(void *args);
// void MC_Motors(void *args);

/*************FUNCTION*************/
esp_err_t xResetMotors(void);
esp_err_t xClearCounter(xSCRP_motor_t *SCRP_motor);
esp_err_t xStopMotor(xSCRP_motor_t *SCRP_motor);
esp_err_t xUpdateMotor(xSCRP_motor_t *SCRP_motor);
esp_err_t xComputeControlSignal(xSCRP_motor_t *SCRP_motor);
esp_err_t xMotorSetup(xSCRP_motor_t *SCRP_motor);
esp_err_t xSetMotor(xSCRP_motor_t *SCRP_motor);