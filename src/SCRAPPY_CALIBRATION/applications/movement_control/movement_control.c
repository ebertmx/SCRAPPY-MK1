
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
#define ALPHAmotorpwm 19
#define ALPHAmotordir 21
#define ALPHAmotorenc 18

#define BETAmotorpwm 26
#define BETAmotordir 27
#define BETAmotorenc 25

#define CHARLIEmotorpwm 16
#define CHARLIEmotordir 17
#define CHARLIEmotorenc 4

// Queues
QueueHandle_t xMC_queue;
xQueueHandle enc_evt_queue;

// Motor Structs
static xSCRP_motor_t ALPHA_motor = {
    .pin_pwm = ALPHAmotorpwm,
    .pin_dir = ALPHAmotordir,
    .pin_enc = ALPHAmotorenc,
    .speed = 0,
    .direction = 0,
    .position = 0,
    .moving = false,
    .pwm_io = MCPWM0A,
    .pwm_unit = MCPWM_UNIT_0,
    .pwm_timer = MCPWM_TIMER_0,
    .pwm_gen = MCPWM_OPR_A,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_0,
    .enc_channel = PCNT_CHANNEL_0};

static xSCRP_motor_t BETA_motor = {
    .pin_pwm = BETAmotorpwm,
    .pin_dir = BETAmotordir,
    .pin_enc = BETAmotorenc,
    .speed = 0,
    .direction = 0,
    .position = 0,
    .moving = false,
    .pwm_io = MCPWM0B,
    .pwm_unit = MCPWM_UNIT_1,
    .pwm_timer = MCPWM_TIMER_0,
    .pwm_gen = MCPWM_OPR_B,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_1,
    .enc_channel = PCNT_CHANNEL_0};

static xSCRP_motor_t CHARLIE_motor = {
    .pin_pwm = CHARLIEmotorpwm,
    .pin_dir = CHARLIEmotordir,
    .pin_enc = CHARLIEmotorenc,
    .speed = 0,
    .direction = 0,
    .position = 0,
    .pwm_io = MCPWM2A,
    .pwm_unit = MCPWM_UNIT_0,
    .pwm_timer = MCPWM_TIMER_2,
    .pwm_gen = MCPWM_OPR_B,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_2,
    .enc_channel = PCNT_CHANNEL_0};

// Variables
bool pcnt_isr_installed = false;

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
    portBASE_TYPE res;

    ESP_LOGI(MCTAG, "Starting Movement Controller");
    ESP_LOGI(MCTAG, "Setting up motors");
    xMotorSetUp(&ALPHA_motor);
    xMotorSetUp(&BETA_motor);
    xMotorSetUp(&CHARLIE_motor);

    int16_t rxbuff[6];
    static int16_t alpha_target = 0;
    static int16_t beta_target = 0;
    static int16_t charlie_target = 0;
    static int16_t alpha_speed = 0;
    static int16_t beta_speed = 0;
    static int16_t charlie_speed = 0;
    // BETA_motor.position = 0;

    // Calibrate


    while (1)
    {
        // Wait for position instructions
        if (xQueueReceive(xMC_queue, &(rxbuff), portMAX_DELAY))
        {
            // int16_t alpha_target = rxbuff[0];
            ESP_LOGI(MCTAG, "Data Recieved: %d, %d, %d, %d, %d, %d ", rxbuff[0], rxbuff[1], rxbuff[2], rxbuff[3], rxbuff[4], rxbuff[5]);
            alpha_target = rxbuff[0] - ALPHA_motor.position;
            beta_target = rxbuff[1] - BETA_motor.position;
            charlie_target = rxbuff[2] - CHARLIE_motor.position;
            alpha_speed = rxbuff[3];
            beta_speed = rxbuff[4];
            charlie_speed = rxbuff[5];

            ESP_LOGI(MCTAG, "Setting Enc Target Ticks= %d, %d, %d", alpha_target, beta_target, charlie_target);
            xSetTarget(&ALPHA_motor, alpha_target);
            xSetTarget(&BETA_motor, beta_target);
            xSetTarget(&CHARLIE_motor, charlie_target);

            ESP_LOGI(MCTAG, "Absolute Position %d, %d, %d", xGetPosition(&ALPHA_motor), xGetPosition(&BETA_motor), xGetPosition(&CHARLIE_motor));

            if (alpha_target < 0)
            {
                ESP_ERROR_CHECK(xSetMotorDir(&ALPHA_motor, dir_neg));
            }
            else
            {
                ESP_ERROR_CHECK(xSetMotorDir(&ALPHA_motor, dir_pos));
            }

            if (beta_target < 0)
            {
                ESP_ERROR_CHECK(xSetMotorDir(&BETA_motor, dir_neg));
            }
            else
            {
                ESP_ERROR_CHECK(xSetMotorDir(&BETA_motor, dir_pos));
            }

            if (charlie_target < 0)
            {
                ESP_ERROR_CHECK(xSetMotorDir(&CHARLIE_motor, dir_neg));
            }
            else
            {
                ESP_ERROR_CHECK(xSetMotorDir(&CHARLIE_motor, dir_pos));
            }

            ESP_ERROR_CHECK(xSetMotorSpeed(&ALPHA_motor, alpha_speed));
            ESP_ERROR_CHECK(xSetMotorSpeed(&BETA_motor, beta_speed));
            ESP_ERROR_CHECK(xSetMotorSpeed(&CHARLIE_motor, charlie_speed));

            while (BETA_motor.moving) //| ALPHA_motor.moving)// | CHARLIE_motor.moving)
            {

                // ESP_LOGI(MCTAG, "unit = %d", evt.unit);
                // ESP_LOGI(MCTAG, "number of messages %d", uxQueueMessagesWaiting(enc_evt_queue));
                res = xQueueReceive(enc_evt_queue, &evt, 1000 / portTICK_PERIOD_MS);
                // ESP_LOGI(MCTAG, "number of messages %d", uxQueueMessagesWaiting(enc_evt_queue));
                // ESP_LOGI(MCTAG, "res = %d", res);
                if (res == pdTRUE)
                {

                    if ((evt.unit == ALPHA_motor.enc_unit) && ALPHA_motor.moving)
                    {
                        // ESP_LOGI(MCTAG, "ALPHA:Target Reached: %d, SPEED = %f", xGetEncoderValue(&ALPHA_motor), mcpwm_get_duty(ALPHA_motor.pwm_unit, ALPHA_motor.pwm_timer, ALPHA_motor.pwm_gen));

                        ESP_ERROR_CHECK(xStopMotor(&ALPHA_motor));
                        ESP_ERROR_CHECK(xUpdatePosition(&ALPHA_motor));

                        // ESP_ERROR_CHECK(xSetMotorSpeed(&BETA_motor, 100));
                        // ESP_LOGI(MCTAG, "ALPHA:Target Reached: %d, SPEED = %f", xGetEncoderValue(&ALPHA_motor), mcpwm_get_duty(ALPHA_motor.pwm_unit, ALPHA_motor.pwm_timer, ALPHA_motor.pwm_gen));
                        ESP_LOGI(MCTAG, "ALPHA: Target Reached. ABS Position %d, %d", xGetPosition(&ALPHA_motor), ALPHA_motor.position);
                    }
                    if ((evt.unit == BETA_motor.enc_unit) && BETA_motor.moving)
                    {
                        //  ESP_LOGI(MCTAG, "BETA: SPEED = %f", BETA_motor.speed);
                        // ESP_LOGI(MCTAG, "BETA: Target Reached: %d, SPEED = %f", xGetEncoderValue(&BETA_motor), mcpwm_get_duty(BETA_motor.pwm_unit, BETA_motor.pwm_timer, BETA_motor.pwm_gen));
                        // ESP_LOGI(MCTAG, "Evt : %d", evt.status);
                        ESP_ERROR_CHECK(xStopMotor(&BETA_motor));
                        ESP_ERROR_CHECK(xUpdatePosition(&BETA_motor));
                        // ESP_ERROR_CHECK(xSetMotorSpeed(&BETA_motor, 50));
                        // ESP_LOGI(MCTAG, "BETA: Target Reached: %d, SPEED = %f", xGetEncoderValue(&BETA_motor), mcpwm_get_duty(BETA_motor.pwm_unit, BETA_motor.pwm_timer, BETA_motor.pwm_gen));
                        ESP_LOGI(MCTAG, "BETA: Target Reached. ABS Position= %d", BETA_motor.position);
                    }

                    if ((evt.unit == CHARLIE_motor.enc_unit) && CHARLIE_motor.moving)
                    {
                        //  ESP_LOGI(MCTAG, "BETA: SPEED = %f", BETA_motor.speed);

                        ESP_ERROR_CHECK(xStopMotor(&CHARLIE_motor));
                        ESP_ERROR_CHECK(xUpdatePosition(&CHARLIE_motor));

                        // ESP_ERROR_CHECK(xSetMotorSpeed(&BETA_motor, 50));
                        ESP_LOGI(MCTAG, "CHARLIE: Target Reached. ABS Position %d, %d", xGetPosition(&CHARLIE_motor), CHARLIE_motor.position);
                    }
                    res = pdFALSE;
                } // if
                else
                {
                    res = pdFALSE;
                    //  ESP_LOGI(MCTAG, "Current counter value :%d", xGetEncoderValue(&ALPHA_motor));
                } // else
            }

        } // whileforever
    }     // SCRP_MovementControl
}
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
        .duty_mode = MCPWM_DUTY_MODE_0, // change!!!!!!!
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
    if (!pcnt_isr_installed)
    {
        pcnt_isr_service_install(0);
        pcnt_isr_installed = true;
    }
    pcnt_isr_handler_add(SCRP_motor->enc_unit, encoder_isr_handler, (void *)SCRP_motor->enc_unit);
    /* Everything is set up, now go to counting */
    // pcnt_counter_resume(SCRP_motor->enc_unit);

    // set up direction and speed
    ESP_ERROR_CHECK(xSetMotorDir(SCRP_motor, SCRP_motor->direction));
    ESP_ERROR_CHECK(xSetMotorSpeed(SCRP_motor, SCRP_motor->speed));
    return ESP_OK;
}

esp_err_t xSetMotorSpeed(struct xSCRP_motor_t *SCRP_motor, float speed)
{
    SCRP_motor->speed = speed;
    // printf("motor speed= %f\n", SCRP_motor->speed);
    SCRP_motor->moving = true;
    pcnt_counter_resume(SCRP_motor->enc_unit);
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
    // ESP_LOGI(MCTAG, "Get Encoder Value %d", encoder_count);
    return encoder_count;
}

esp_err_t xStopMotor(struct xSCRP_motor_t *SCRP_motor)
{
    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, 0));
    // ESP_ERROR_CHECK(mcpwm_set_signal_high(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen));
    pcnt_counter_pause(SCRP_motor->enc_unit);
    SCRP_motor->moving = false;
    SCRP_motor->speed = 0;
    // ESP_LOGI(MCTAG,"STOPPED MOTOR = %f", SCRP_motor->speed);
    return ESP_OK;
}

esp_err_t xResumeMotor(struct xSCRP_motor_t *SCRP_motor)
{
    pcnt_counter_resume(SCRP_motor->enc_unit);
    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, SCRP_motor->speed));
    SCRP_motor->moving = false;

    return ESP_OK;
}

esp_err_t xSetTarget(struct xSCRP_motor_t *SCRP_motor, int16_t target)
{
    // Update absolute position
    // printf("abs position= %d\n", SCRP_motor->position);
    xUpdatePosition(SCRP_motor);
    //  SCRP_motor->position = SCRP_motor->position + xGetEncoderValue(SCRP_motor);
    // printf("abs position= %d\n", SCRP_motor->position);
    pcnt_set_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, target);
    pcnt_counter_pause(SCRP_motor->enc_unit);
    pcnt_counter_clear(SCRP_motor->enc_unit);
    int16_t eventvalue;
    pcnt_get_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, &eventvalue);
    // ESP_LOGI(MCTAG, "Set Target %d", eventvalue);
    return ESP_OK;
}

int16_t xGetPosition(struct xSCRP_motor_t *SCRP_motor)
{

    // ESP_LOGI(MCTAG, "Get Position %d", xGetEncoderValue(SCRP_motor));

    return (SCRP_motor->position + xGetEncoderValue(SCRP_motor));
}

esp_err_t xUpdatePosition(struct xSCRP_motor_t *SCRP_motor)
{
    // Update the absolute position with the encoder value
    SCRP_motor->position = SCRP_motor->position + xGetEncoderValue(SCRP_motor);
    // set the new target to home position and clear encoder
    // pcnt_set_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, 0);
    pcnt_counter_pause(SCRP_motor->enc_unit);
    pcnt_counter_clear(SCRP_motor->enc_unit);
    // ESP_LOGI(MCTAG, "Updated Position %d", SCRP_motor->position);
    return ESP_OK;
}