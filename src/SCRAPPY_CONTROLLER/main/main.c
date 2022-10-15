/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "main.h"

// GLOBALS
extern TaskHandle_t xh_MC;
extern QueueHandle_t xMC_queue;

void app_main(void)
{
    esp_log_level_set("MC-MOTOR", ESP_LOG_NONE);
    ESP_LOGI(SCRP, "SCRAPPY Starting...");

    // esp_log_level_set("*", ESP_LOG_NONE);//disables logging
    ESP_LOGI(SCRP, "Initiating Startup: SCRAPPY");

    // SET UP NETWORK
    // ESP_LOGI(WIFI, "Starting Wifi...");
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);
    // wifi_init_sta();
    // ESP_LOGI(WIFI, "Wifi Started");

    // ESP_LOGI(NET, "Connecting to Network...");
    // int sock = connect_to_server();
    // if (sock < 0)
    // {
    //     ESP_LOGE(NET, "Connection to server failed");
    // }
    // else
    // {
    //     ESP_LOGI(NET, "Connection successful");
    // }

    ESP_LOGI(SCRP, "Creating Tasks...");
    xTaskCreate(SCRP_MovementControl, "MC", 4086, NULL, 1, &xh_MC);
    xMC_queue = xQueueCreate(10, sizeof(int16_t[7]));
    int count = 0;

    ESP_LOGI(SCRP, "Standing by...");

    int16_t myposition1[] = {'P', -100, -100, -100, 75, 30, 75};
    int16_t myposition2[] = {'P', 100, 100, -100, 50, 30, 60};
    char command[128];
    char args[16][16];
    int16_t intArgs[7];
    int numArgs = 0;
    int i = 0;

    // while (1)
    // {
    //     int len = recv(sock, command, sizeof(command) - 1, 0);
    //     if (len > 0)
    //     {
    //         command[len] = 0; // Null-terminate whatever we received and treat like a string
    //         ESP_LOGI(SCRP, "Received %d bytes from %s:", len, HOST_IP_ADDR);
    //         ESP_LOGI(SCRP, "%s", command);
    //         numArgs = parseCommand(command, len, args);
    //         if (strcmp(command, "move") == 0)
    //         {
    //             intArgs[0] = 'P';
    //             convertToInts(args, numArgs, intArgs);
    //             xQueueSendToBack(xMC_queue, (void *)&(intArgs), portMAX_DELAY);
    //         }
    //     }
    //     vTaskDelay(10 / portTICK_RATE_MS);
    // }

    while (i < 5)
    {
        ESP_LOGI(SCRP, "SENDING...");
        xQueueSendToBack(xMC_queue, (void *)&(myposition2), portMAX_DELAY);
        myposition2[1] += 100;
        myposition2[2] += 100;
        myposition2[3] += 100;
        vTaskDelay(100 / portTICK_RATE_MS);
        ESP_LOGI(SCRP, "SENDING...");
        xQueueSendToBack(xMC_queue, (void *)&(myposition1), portMAX_DELAY);
        myposition2[1] += -100;
        myposition2[2] += -100;
        myposition2[3] += -100;

        vTaskDelay(10 / portTICK_RATE_MS);
        i++;
    }

    while (1)
    {
        vTaskDelay(3000 / portTICK_RATE_MS);
        ESP_LOGI(SCRP, "Standing by...");
    }
}

/**************************WIFIANDNETWORK**************************/

void tcp_client(void);

void convertToInts(char args[16][16], int numArgs, int16_t intArgs[])
{
    char *ptr;
    for (int i = 1; i <= numArgs; i++)
    {
        intArgs[i] = (int16_t)(strtol(args[i], &ptr, 10));
    }
}

int parseCommand(char *command, int len, char args[][16])
{
    int parseArgs = 0;
    int curArg = 0;
    int argIndex = 0;
    char c;
    for (int i = 0; i < len; i++)
    {
        c = command[i];
        if (c == ':')
        {
            command[i] = '\0';
            parseArgs = 1;
            continue;
        }
        if (parseArgs)
        {
            if (c == ',')
            {
                curArg++;
                argIndex = 0;
                continue;
            }
            args[curArg][argIndex++] = c;
        }
    }
    return curArg + parseArgs;
}

int connect_to_server()
{
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_STREAM, 0);
    int flags = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    if (sock < 0)
    {
        ESP_LOGE(NET, "Unable to create socket: errno %d", errno);
        return -1;
    }
    ESP_LOGI(NET, "Socket created, connecting to %s:%d", host_ip, PORT);

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    return sock;
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIFI, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(WIFI, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            //  .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(WIFI, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(WIFI, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(WIFI, "UNEXPECTED EVENT");
    }
}