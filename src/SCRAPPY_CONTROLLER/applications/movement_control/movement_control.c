
/*  Created: 2022-10-02 By: ebertmx

*Description: This task operates and monitors the movement operations
    of SCRAPPY, a robot arm. It is solely responsible to driving the motors
    and reading sensor data. All other processes must use the
    SCRP_MovementControl
    task to access SCRAPPY's motors
*A list of function and components can be found in movement_control.h
*/
#include "movement_control.h"
// Tasks
TaskHandle_t xh_MC = NULL;

TaskHandle_t xh_MC_Set = NULL;
TaskHandle_t xh_MC_Run = NULL;
TaskHandle_t xh_MC_Stop = NULL;
TaskHandle_t xh_MC_Motors = NULL;
// Queues
QueueHandle_t xMC_queue;
xQueueHandle enc_evt_queue;

// Motor Structs
static xSCRP_motor_t MOTOR0 = {
    .num = 0,
    .pin_pwm = motorpwm0,
    .pin_dir = motordir0,
    .pin_enc = motorenc0,
    .speed = 0,
    .signal = 0,
    .direction = 0,
    .position = 0,
    .target = 0,
    .enc_target = 0,
    .enable = false,
    .pwm_io = MCPWM0A,
    .pwm_unit = MCPWM_UNIT_0,
    .pwm_timer = MCPWM_TIMER_0,
    .pwm_gen = MCPWM_OPR_A,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_0,
    .enc_channel = PCNT_CHANNEL_0};

static xSCRP_motor_t MOTOR1 = {
    .num = 1,
    .pin_pwm = motorpwm1,
    .pin_dir = motordir1,
    .pin_enc = motorenc1,
    .speed = 0,
    .signal = 0,
    .direction = 0,
    .position = 0,
    .target = 0,
    .enc_target = 0,
    .enable = false,
    .pwm_io = MCPWM0B,
    .pwm_unit = MCPWM_UNIT_1,
    .pwm_timer = MCPWM_TIMER_0,
    .pwm_gen = MCPWM_OPR_B,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_1,
    .enc_channel = PCNT_CHANNEL_0};

static xSCRP_motor_t MOTOR2 = {
    .num = 2,
    .pin_pwm = motorpwm2,
    .pin_dir = motordir2,
    .pin_enc = motorenc2,
    .speed = 0,
    .signal = 0,
    .direction = 0,
    .position = 0,
    .target = 0,
    .enc_target = 0,
    .enable = false,
    .pwm_io = MCPWM2B,
    .pwm_unit = MCPWM_UNIT_0,
    .pwm_timer = MCPWM_TIMER_2,
    .pwm_gen = MCPWM_OPR_B,
    .enc_dir = PCNT_COUNT_DEC,
    .enc_unit = PCNT_UNIT_2,
    .enc_channel = PCNT_CHANNEL_0};

// Variables
static bool pcnt_isr_installed = false;
static bool STOP = false;
// INTERRUPTS
static void IRAM_ATTR encoder_isr_handler(void *arg)
{
    // ESP_LOGI(MCTAG, "Interrupt");
    int pcnt_unit = (int)arg;
    /* Save the PCNT event type that caused an interrupt
       to pass it to the main program */
    // pcnt_get_event_status(pcnt_unit, &evt.status);
    // printf("Unit: %d    Event: %c", evt.unit, evt.status);
    switch (pcnt_unit)
    {
    case 0:
        MOTOR0.enable = false;
        break;
    case 1:
        MOTOR1.enable = false;
        break;
    case 2:
        MOTOR2.enable = false;
        break;
    }
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(xh_MC_Stop, &xHigherPriorityTaskWoken);
}

//*******************TASKS************************//
/**
 * @brief : Handles Messages and Commands from SCRP main
 *
 * @param args : Nothing
 */
void SCRP_MovementControl(void *args)
{
    ESP_LOGI(MC, "Initiate SCRP Movement Controller");
    enc_evt_t evt;
    enc_evt_queue = xQueueCreate(10, sizeof(enc_evt_t));

    static int16_t rxbuff[7];

    ESP_LOGI(MC, "Initiate Motor Setup");
    ESP_ERROR_CHECK(xMotorSetup(&MOTOR0));
    ESP_ERROR_CHECK(xMotorSetup(&MOTOR1));
    ESP_ERROR_CHECK(xMotorSetup(&MOTOR2));
    ESP_LOGI(MC, "Motor Setup Complete");

    ESP_LOGI(MC, "Creating Tasks");
    xTaskCreatePinnedToCore(MC_Run, "MC_Run", 4086, NULL, tskIDLE_PRIORITY, &xh_MC_Run, 1);
    xTaskCreate(MC_Stop, "MC_Stop", 2048, NULL, 2, &xh_MC_Stop);

    ESP_LOGI(MC, "Standing by...");

    while (1)
    {

        if (xQueueReceive(xMC_queue, &(rxbuff), portMAX_DELAY))
        {
            char code = rxbuff[0];

            ESP_LOGI(MC, "Data Recieved: %c, %d, %d, %d, %d, %d, %d ", code,
                     rxbuff[1],
                     rxbuff[2],
                     rxbuff[3],
                     rxbuff[4],
                     rxbuff[5],
                     rxbuff[6]);

            switch (code)
            {
            case 'P':
                vTaskDelay(10 / portTICK_RATE_MS);
                if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
                {

                    MOTOR0.target = rxbuff[1];
                    MOTOR1.target = rxbuff[2];
                    MOTOR2.target = rxbuff[3];// - rxbuff[2]; // compensate for link1 rotation

                    MOTOR0.speed = rxbuff[4];
                    MOTOR1.speed = rxbuff[5];
                    MOTOR2.speed = rxbuff[6];

                    xSetMotor(&MOTOR0);
                    xSetMotor(&MOTOR1);
                    xSetMotor(&MOTOR2);
                    xTaskNotifyGive(xh_MC_Run);
                }
                break;

            case 'S':
                xTaskNotifyGive(xh_MC_Stop);
                break;

            case 'C':
                break;
            default:
                break;
            }
        }
    }
}

/**
 * @brief : Computes control signals and runs motors based on target position.
 *          Starts encoder count when activated
 *
 * @param args : Nothing
 */
void MC_Run(void *args)
{
    ESP_LOGI(MCRUN, "Initialized...");
    while (1)
    {
        // tell MC ready for task
        xTaskNotifyGive(xh_MC);
        // wait until MC notifies task is available
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
        {
            ESP_LOGI(MCRUN, "Updated Target: P=%d, %d, %d;  S = %d, %d, %d", MOTOR0.target, MOTOR1.target, MOTOR2.target, MOTOR0.speed, MOTOR1.speed, MOTOR2.speed);
            int count = 0;
            MOTOR0.enable = true;
            MOTOR1.enable = true;
            MOTOR2.enable = true;
            while (MOTOR0.enable || MOTOR1.enable || MOTOR2.enable)
            {
                if (MOTOR0.enable)
                {
                    xComputeControlSignal(&MOTOR0);
                    xUpdateMotor(&MOTOR0);
                }
                else
                {
                    xStopMotor(&MOTOR0);
                }
                if (MOTOR1.enable)
                {
                    xComputeControlSignal(&MOTOR1);
                    xUpdateMotor(&MOTOR1);
                }
                else
                {
                    xStopMotor(&MOTOR1);
                }
                if (MOTOR2.enable)
                {
                    xComputeControlSignal(&MOTOR2);
                    xUpdateMotor(&MOTOR2);
                }
                else
                {
                    xStopMotor(&MOTOR2);
                }
            }
            xStopMotor(&MOTOR0);
            xStopMotor(&MOTOR1);
            xStopMotor(&MOTOR2);
        }
    }
}

/**
 * @brief : Halts Motors, stop encoder count, and reports status. Suspends MC_Run on activation
 *
 * @param args : Nothing
 */
void MC_Stop(void *args)
{
    ESP_LOGI(MCSTOP, "Initialized...");
    while (1)
    {
        // ulTaskNotifyTakeIndexed(3, pdTRUE, portMAX_DELAY)
        // {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
        {
            if (!MOTOR0.enable)
            {
                xStopMotor(&MOTOR0);
                ESP_LOGI(MCSTOP, "Stop Triggered on MOTOR0");
            }
            if (!MOTOR1.enable)
            {
                xStopMotor(&MOTOR1);
                ESP_LOGI(MCSTOP, "Stop Triggered on MOTOR1");
            }
            if (!MOTOR2.enable)
            {
                xStopMotor(&MOTOR2);
                ESP_LOGI(MCSTOP, "Stop Triggered on MOTOR2");
            }
        }
        // if (ulTaskNotifyTakeIndexed(1, pdTRUE, 0))
        // {
        //     xStopMotor(&MOTOR1);
        //     ESP_LOGI(MCSTOP, "Stop Triggered");
        // }
        // if (ulTaskNotifyTakeIndexed(2, pdTRUE, 0))
        // {
        //     xStopMotor(&MOTOR2);
        //     ESP_LOGI(MCSTOP, "Stop Triggered");
        // }
        // }
    }
}

// /**
//  * @brief : Sets up SCRP motors. Updates settable parameters on request.
//  *
//  * @param args : Nothing
//  */
// void MC_Motors(void *args)
// {
// }

// /**
//  * @brief : Set encoder targets based on current and desired positions. Call MC_Motors to update
//  *
//  * @param args : Nothing
//  */
// void MC_Set(void *args)
// {
// }

//****************************//

// FUNCTIONS
esp_err_t xStopMotor(xSCRP_motor_t *SCRP_motor)
{
    // SCRP_motor->enable = false;
    pcnt_counter_pause(SCRP_motor->enc_unit);
    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, 0));
    return ESP_OK;
}

esp_err_t xUpdateMotor(xSCRP_motor_t *SCRP_motor)
{
    ESP_LOGI(MCMOTORS, "Updating MOTOR%d...", SCRP_motor->num);

    // SET DIRECTION
    gpio_set_level(SCRP_motor->pin_dir, SCRP_motor->direction);

    if (DIRPOS == SCRP_motor->direction)
    {
        SCRP_motor->enc_dir = PCNT_COUNT_INC;
    }
    else
    {
        SCRP_motor->enc_dir = PCNT_COUNT_DEC;
    }
    pcnt_set_mode(SCRP_motor->enc_unit, SCRP_motor->enc_channel, SCRP_motor->enc_dir, PCNT_COUNT_DIS, PCNT_MODE_KEEP, PCNT_MODE_KEEP);
    pcnt_counter_resume(SCRP_motor->enc_unit); // start counter if not started

    // SET SIGNAL
    ESP_ERROR_CHECK(mcpwm_set_duty(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, SCRP_motor->pwm_gen, SCRP_motor->signal));
    ESP_LOGI(MCMOTORS, "Updated MOTOR%d to duty = %f, dir = %d", SCRP_motor->num, SCRP_motor->signal, SCRP_motor->direction);
    return ESP_OK;
}
esp_err_t xComputeControlSignal(xSCRP_motor_t *SCRP_motor)
{
    ESP_LOGI(MCMOTORS, "Computing MOTOR%d control signal...", SCRP_motor->num);
    int16_t encoder;
    pcnt_get_counter_value(SCRP_motor->enc_unit, &encoder);

    int16_t error = SCRP_motor->enc_target - encoder;

    if (error < 0)
    {
        SCRP_motor->direction = DIRNEG;
    }
    else
    {
        SCRP_motor->direction = DIRPOS;
    }
    // 0.01 * SCRP_motor->speed
    if (abs(encoder) < abs(SCRP_motor->enc_target / 2))
    {
        SCRP_motor->signal = 200 * (abs(encoder));
    }
    else
    {
        SCRP_motor->signal = 200 * (abs(error));
    }
    SCRP_motor->signal /= abs(SCRP_motor->enc_target);
    // printf("error = %d", error);
    // printf("signal = %f \n", SCRP_motor->signal);
    if (SCRP_motor->signal > 70)
    {
        SCRP_motor->signal = 70;
    }
    if (SCRP_motor->signal < 30)
    {
        SCRP_motor->signal = 30;
    }
    ESP_LOGI(MCMOTORS, "MOTOR%d Signal =%f", SCRP_motor->num, SCRP_motor->signal);
    return ESP_OK;
}

esp_err_t xSetMotor(xSCRP_motor_t *SCRP_motor)
{
    ESP_LOGI(MC, "Setting MOTOR%d...", SCRP_motor->num);

    // Update Position
    pcnt_counter_pause(SCRP_motor->enc_unit); // make sure encoder is paused
    int16_t encoder_count = 0;
    pcnt_get_counter_value(SCRP_motor->enc_unit, &encoder_count);
    SCRP_motor->position = SCRP_motor->position + encoder_count;
    ESP_LOGI(MC, "MOTOR%d position=%d", SCRP_motor->num, SCRP_motor->position);
    pcnt_counter_clear(SCRP_motor->enc_unit); // clear counter

    // set encoder target
    SCRP_motor->enc_target = SCRP_motor->target - SCRP_motor->position;

    // set interrupt trigger
    pcnt_set_event_value(SCRP_motor->enc_unit, PCNT_EVT_THRES_1, SCRP_motor->enc_target);
    pcnt_counter_clear(SCRP_motor->enc_unit); // clear counter to enable trigger

    ESP_LOGI(MC, "MOTOR%d set; target = %d", SCRP_motor->num, SCRP_motor->enc_target);
    return ESP_OK;
}

esp_err_t xMotorSetup(xSCRP_motor_t *SCRP_motor)
{

    // MOTOR

    ESP_LOGI(MCMOTORS, "Configuring MOTOR%d PWM...", SCRP_motor->num);

    ESP_ERROR_CHECK(mcpwm_gpio_init(SCRP_motor->pwm_unit, SCRP_motor->pwm_io, SCRP_motor->pin_pwm)); // pwm pin

    mcpwm_config_t pwm_config_motor = {
        .frequency = PWMFREQUENCY,
        .cmpr_a = SCRP_motor->speed,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = DUTYMODE, // change!!!!!!!
    };

    mcpwm_init(SCRP_motor->pwm_unit, SCRP_motor->pwm_timer, &pwm_config_motor);

    gpio_set_direction(SCRP_motor->pin_dir, GPIO_MODE_OUTPUT);

    // gpio_set_level(motor->pin_dir, motor->dir);

    // Encoder
    ESP_LOGI(MCMOTORS, "Configuring MOTOR%d PCNT...", SCRP_motor->num);
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

    /* Install interrupt service and add isr callback handler */
    if (!pcnt_isr_installed)
    {
        pcnt_isr_service_install(0);
        pcnt_isr_installed = true;
    }
    pcnt_isr_handler_add(SCRP_motor->enc_unit, encoder_isr_handler, (void *)SCRP_motor->enc_unit);
    pcnt_counter_clear(SCRP_motor->enc_unit);
    ESP_LOGI(MCMOTORS, "MOTOR%d Configured", SCRP_motor->num);
    return ESP_OK;
}