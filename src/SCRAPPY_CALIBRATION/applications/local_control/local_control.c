
/*  Created: 2022-10-02 By: ebertmx

*Description: This task calculates and send encoder position to SCRAPPY's movement controller.
    This task deals with XYZ coordinates and the environment. To use this task properly SCRAPPY
    must be in a calibrated state
*A list of function and components can be found in position_control.h
*/
#include "local_control.h"
extern QueueHandle_t xMC_queue;
QueueHandle_t xLC_queue;

// INTERRUPTS
static void IRAM_ATTR button_handler_isr(void *arg)
{
    uint32_t pin = (uint32_t)arg;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // uint16_t debounce = 0x00;
    // for(int i = 0; i<16; i++){
    //     debouce = debounce | gpio_get_level(pin);
    // }
    xQueueSendFromISR(xLC_queue, &pin, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

//************ MAIN TASK ************//

void SCRP_LocalController(void *argv)
{
    // set up pins
    ESP_LOGI(LCTAG, "Starting Local Controller");

    gpio_config_t button = {
        .intr_type = GPIO_PIN_INTR_NEGEDGE,
        .pin_bit_mask = GPIO_BIT_MASK,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0};
    gpio_config(&button);
    // gpio_set_intr_type(pin1, GPIO_INTR_ANYEDGE);
    // gpio_set_intr_type(pin2, GPIO_INTR_ANYEDGE);
    // gpio_set_intr_type(pin3, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin1, button_handler_isr, (void *)pin1);
    // gpio_isr_handler_add(pin2, button_handler_isr, (void *)pin2);
    gpio_isr_handler_add(pin3, button_handler_isr, (void *)pin3);

    uint32_t pin;

    ESP_LOGI(LCTAG, "Waiting for Input");
    int16_t myposition[] = {-1, -1, -1};
    int dir1 = 1;
    int dir3 = -1;
    int count =0;;
    while (1)
    {
        if (xQueueReceive(xLC_queue, &pin, portMAX_DELAY))

        {
            vTaskDelay(50 / portTICK_RATE_MS);
            // ESP_LOGI(LCTAG, "Got Signal From Pin: %d ", pin);

            if (pin1 == pin)
            {
                // myposition[0] += dir1 * (100);
                // ESP_LOGI(LCTAG, "Sent: y = %d", myposition[1]);
                xQueueSend(xMC_queue, (void *)&(myposition), 0);
                // xQueueSend(xMC_queue, (void *)&(myposition.y), 0);
                // xQueueSend(xMC_queue, (void *)&(myposition.z), 0);
                ESP_LOGI(LCTAG, "myposition %d, %d, %d", myposition[0], myposition[1], myposition[2]);
                if (count % 2 == 0)
                {
                    if (myposition[0] == 1)
                    {

                        myposition[0] = -1;
                        myposition[1] = -1;
                        myposition[2] = -1;
                    }
                    else
                    {

                        myposition[0] = 1;
                        myposition[1] = 1;
                        myposition[2] = 1;
                    }
                }
            }

            // if (pin3 == pin)
            // {
            //     // myposition[1]+= dir3 * (100);
            //     // ESP_LOGI(LCTAG, "Sent: z = %d ", myposition[1]);
            //     xQueueSend(xMC_queue, (void *)&(myposition), 0);
            //     if (myposition[1] == 1)
            //     {
            //         myposition[1] = -1;
            //     }
            //     if (myposition[1] == -1)
            //     {
            //         myposition[1] = 1;
            //     }
            // }
        }
        count=count+1;
    }
    // xQueueSend(xPC_queue, (void *)&(myposition), 0);
    return;
}
