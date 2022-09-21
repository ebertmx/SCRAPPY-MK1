
#include "camera1.h"
void func(void)
{
}

#define LOG_TAG "camera1"

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000, // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_UXGA,   // QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12,                 // 0-63 lower number means higher quality
    .fb_count = 1,                      // if more than one, i2s runs in continuous mode. Use only with JPEG
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY // CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

esp_err_t camera_init(void)
{
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGI(LOG_TAG, "Camera Init Failed");
        return err;
    }
    return ESP_OK;
}

esp_err_t camera_capture(size_t *_jpg_buf_len, uint8_t **_jpg_buf)
{
    // acquire a frame

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        ESP_LOGI(LOG_TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    if (fb->format != PIXFORMAT_JPEG)
    {

        bool jpeg_converted = frame2jpg(fb, 80, _jpg_buf, _jpg_buf_len);
        if (!jpeg_converted)
        {
            ESP_LOGE(LOG_TAG, "JPEG compression failed");
            esp_camera_fb_return(fb);
            return ESP_FAIL;
        }
    }
    else
    {

        *_jpg_buf_len = fb->len;
        *_jpg_buf = fb->buf;
    }
    esp_camera_fb_return(fb);

    return ESP_OK;
}