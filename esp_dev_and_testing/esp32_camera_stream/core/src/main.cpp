#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#define LOG_TAG "MAIN"

extern "C" void app_main(void)
{
    static httpd_handle_t server = NULL;
    WIFI::Wifi Wifi;

    ESP_LOGI(LOG_TAG, "Creating default event loop");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(LOG_TAG, "Initialising NVS");
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_err_t status{ESP_OK};

    ESP_LOGI(LOG_TAG, "Initializing Wifi...");
    status |= Wifi.init();

    
    ESP_LOGI(LOG_TAG, "Initializing Camera...");
    status |= camera_init();
    ESP_LOGI(LOG_TAG, "Camera Initialized...");


    if (ESP_OK == status)
    {
        status |= Wifi.begin();
    }


    if (ESP_OK == status)
    {
        ESP_LOGI(LOG_TAG, "Starting Server in Setup");

        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

        server = start_webserver();
        ESP_LOGI(LOG_TAG, "Started server");
    }
    else
    {
        ESP_LOGI(LOG_TAG, "Camera Failed");
    }
}
