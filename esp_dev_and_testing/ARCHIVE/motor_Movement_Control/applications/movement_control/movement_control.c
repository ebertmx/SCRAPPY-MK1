
/*  Created: 2022-10-02 By: ebertmx

*Description: This task operates and monitors the movement operations
    of SCRAPPY, a robot arm. It is solely responsible to driving the motors
    and reading sensor data. All other processes must use the
    SCRP_MovementControl
    task to access SCRAPPY's motors
*A list of function and components can be found in movement_control.h
*/
#include "movement_control.h"

// PIN DEFs
#define ALPHAmotorpwm 26
#define ALPHAmotordir 27
#define ALPHAmotorenc 25

// Queues
QueueHandle_t xMC_queue;
xQueueHandle enc_evt_queue;

// Motor Structs
xSCRP_motor_t ALPHA_motor = {
    .pin_pwm = ALPHAmotorpwm,
    .pin_dir = ALPHAmotordir,
    .pin_enc = ALPHAmotorenc,
    .speed = 0,
    .direction = 0,
    .position = 0,
    .pwm_io = MCPWM0A,
    .pwm_unit = MCPWM_UNIT_0,
    .pwm_timer = MCPWM_TIMER_0,
    .pwm_gen = MCPWM_OPR_A,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_0,
    .enc_channel = PCNT_CHANNEL_0};

// Variables

// INTERRUPTS
static void IRAM_ATTR encoder_isr_handler(void *arg)
{
    // ESP_LOGI(MCTAG, "Interrupt");
    int pcnt_unit = (int)arg;
    enc_evt_t evt;
    evt.unit = pcnt_unit;
    /* Save the PCNT event type that caused an interrupt
       to pass it to the main program */
    pcnt_get_event_status(pcnt_unit, &evt.status);
    // printf("Unit: %d    Event: %c", evt.unit, evt.status);
    xQueueSendFromISR(enc_evt_queue, &evt, NULL);
}

//******************* MAIN TASK ************************//

void SCRP_MovementControl(void *args)
{

    enc_evt_queue = xQueueCreate(10, sizeof(enc_evt_t));
    enc_evt_t evt;

    ESP_LOGI(MCTAG, "Starting Movement Controller");
    ESP_LOGI(MCTAG, "Setting up motors");
    xMotorSetUp(&ALPHA_motor);

    int16_t rxbuff;
    while (1)
    {
        //Wait for position instructions
        if (xQueueReceive(xMC_queue, &(rxbuff), portMAX_DELAY))
        {
            xSetTarget(&ALPHA_motor, rxbuff);
            ESP_LOGI(MCTAG, "Absolute Position %d", xGetPosition(&ALPHA_motor));
            if (rxbuff < 0)
            {
                ESP_ERROR_CHECK(xSetMotorDir(&ALPHA_motor, dir_neg));
            }
            else
            {
                ESP_ERROR_CHECK(xSetMotorDir(&ALPHA_motor, dir_pos));
            }
            ESP_ERROR_CHECK(xSetMotorSpeed(&ALPHA_motor, 50));

            while (1)
            {
                if (xQueueReceive(enc_evt_queue, &evt, 1000 / portTICK_PERIOD_MS))
                {
                    ESP_ERROR_CHECK(xStopMotor(&ALPHA_motor));

                    ESP_LOGI(MCTAG, "Target Reached: %d", xGetEncoderValue(&ALPHA_motor));
                    break;
                }//if
                else
                {

                  //  ESP_LOGI(MCTAG, "Current counter value :%d", xGetEncoderValue(&ALPHA_motor));
                }//else
            }//while
        }//if
    }//whileforever
}//SCRP_MovementControl

//****************************//

// FUNCTIONS
esp_err_t xMotorSetUp(struct xSCRP_motor_t *SCRP_motor)
{
    // MOTOR

    ESP_ERROR_CHECK(mcpwm_gpio_init(SCRP_motor->pwm_unit, SCRP_motor->pwm_io, SCRP_motor->pin_pwm)); // pwm pin

    mcpwm_config_t pwm_config_motor = {
        .frequency = PWMFREQUENCY,
        .cmpr_a = SCRP_motor->speed,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };

    mcpwm_init(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, &pwm_config_motor);

    gpio_set_direction(SCRP_motor->pin_dir, GPIO_MODE_OUTPUT);

    // gpio_set_level(motor->pin_dir, motor->dir);

    // SENSOR
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = SCRP_motor->pin_enc,
        .ctrl_gpio_num = PCNT_PIN_NOT_USED,
        .channel = SCRP_motor->enc_channel, // PCNT_CHANNEL_0,
        .unit = SCRP_motor->enc_unit,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = SCRP_motor->enc_dir, // Count on the positive edge
        .neg_mode = PCNT_COUNT_DIS,      // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP, // Keep the primary counter mode if high
                                      // Set the maximum and minimum limit values to watch
        .counter_h_lim = motorHlimit,
        .counter_l_lim = motorLlimit,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(SCRP_motor->enc_unit, 100);
    pcnt_filter_enable(SCRP_motor->enc_unit);

    pcnt_set_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, 100);
    pcnt_event_enable(SCRP_motor->enc_unit, PCNT_EVT_THRES_1);

    pcnt_event_disable(SCRP_motor->enc_unit, PCNT_EVT_H_LIM);
    pcnt_event_disable(SCRP_motor->enc_unit, PCNT_EVT_L_LIM);
    pcnt_event_disable(SCRP_motor->enc_unit, PCNT_EVT_ZERO);
    pcnt_event_disable(SCRP_motor->enc_unit, PCNT_EVT_THRES_0);
    /* Initialize PCNT's counter */
    pcnt_counter_pause(SCRP_motor->enc_unit);
    pcnt_counter_clear(SCRP_motor->enc_unit);
    /* Install interrupt service and add isr callback handler */
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(SCRP_motor->enc_unit, encoder_isr_handler, (void *)SCRP_motor->enc_unit);
    /* Everything is set up, now go to counting */
    pcnt_counter_resume(SCRP_motor->enc_unit);

    // set up direction and speed
    //  ESP_ERROR_CHECK( setmotordir(motor,sensor,motor->dir));
    //  ESP_ERROR_CHECK(setmotorspeed(motor,motor->duty));
    return ESP_OK;
}

esp_err_t xSetMotorSpeed(struct xSCRP_motor_t *SCRP_motor, float speed)
{
    SCRP_motor->speed = speed;
    // printf("motor speed= %f\n", SCRP_motor->speed);

    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, SCRP_motor->speed));

    return ESP_OK;
}

esp_err_t xSetMotorDir(struct xSCRP_motor_t *SCRP_motor, direction_t dir)
{
    SCRP_motor->direction = dir;
    gpio_set_level(SCRP_motor->pin_dir, SCRP_motor->direction);
    if (dir_pos == SCRP_motor->direction)
    {
        SCRP_motor->enc_dir = PCNT_COUNT_INC;
    }
    if (dir_neg == SCRP_motor->direction)
    {
        SCRP_motor->enc_dir = PCNT_COUNT_DEC;
    }
    // printf("encoder direction %d\n", SCRP_motor->enc_dir);
    pcnt_set_mode(SCRP_motor->enc_unit, SCRP_motor->enc_channel, SCRP_motor->enc_dir, PCNT_COUNT_DIS, PCNT_MODE_KEEP, PCNT_MODE_KEEP);
    return ESP_OK;
}

int16_t xGetEncoderValue(struct xSCRP_motor_t *SCRP_motor)
{
    int16_t encoder_count;
    pcnt_get_counter_value(SCRP_motor->enc_unit, &encoder_count);
    return encoder_count;
}

esp_err_t xStopMotor(struct xSCRP_motor_t *SCRP_motor)
{
    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, 0));
    return ESP_OK;
}

esp_err_t xResumeMotor(struct xSCRP_motor_t *SCRP_motor)
{
    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, SCRP_motor->speed));
    return ESP_OK;
}

esp_err_t xSetTarget(struct xSCRP_motor_t *SCRP_motor, int16_t target)
{
    // Update absolute position
    SCRP_motor->position = SCRP_motor->position + xGetEncoderValue(SCRP_motor);
    pcnt_set_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, target);
    pcnt_counter_clear(SCRP_motor->enc_unit);
    return ESP_OK;
}

int16_t xGetPosition(struct xSCRP_motor_t *SCRP_motor)
{
    return (SCRP_motor->position + xGetEncoderValue(SCRP_motor));
}

esp_err_t xUpdatePosition(struct xSCRP_motor_t *SCRP_motor)
{
    //Update the absolute position with the encoder value
    SCRP_motor->position = SCRP_motor->position + xGetEncoderValue(SCRP_motor);
    //set the new target to home position and clear encoder
    pcnt_set_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, 0);
    pcnt_counter_clear(SCRP_motor->enc_unit);
    return ESP_OK;
}