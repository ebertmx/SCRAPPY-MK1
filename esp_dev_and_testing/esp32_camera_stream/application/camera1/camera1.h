#include <stdio.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_camera.h"
#include "esp_log.h"

void func(void);

// #define CAM_PIN_PWDN    -1 //power down is not used
// #define CAM_PIN_RESET   -1 //software reset will be performed
// #define CAM_PIN_XCLK    21
// #define CAM_PIN_SIOD    26
// #define CAM_PIN_SIOC    27

// #define CAM_PIN_D7      35
// #define CAM_PIN_D6      34
// #define CAM_PIN_D5      39
// #define CAM_PIN_D4      36
// #define CAM_PIN_D3      19
// #define CAM_PIN_D2      18
// #define CAM_PIN_D1       5
// #define CAM_PIN_D0       4
// #define CAM_PIN_VSYNC   25
// #define CAM_PIN_HREF    23
// #define CAM_PIN_PCLK    22

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22


//static camera_config_t camera_config;

esp_err_t camera_init(void);
esp_err_t camera_capture( size_t* jpg_buf_len, uint8_t **__jpg_buf);



