#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "movement_control.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"

/*********TAGS*****************/
static const char *SCRP = "SCRAPPY";
static const char *WIFI = "SCRP-WIFI";
static const char *NET = "SCRP-NETWORK";

#define INCLUDE_vTaskSuspend 1

/****************NETWORK************/
#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "fcntl.h"

#define HOST_IP_ADDR "192.168.1.140"
#define PORT 9999
#define MAX_ARG_LENGTH 16

/**********WIFI*******************/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID "FREE_WIFI!!!"
#define EXAMPLE_ESP_WIFI_PASS "WarzoneIsLife1"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static int s_retry_num = 0;

// FUNCTIONS
void convertToInts(char args[16][16], int numArgs, int16_t intArgs[]);
int parseCommand(char *command, int len, char args[][16]);
int connect_to_server();

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
void wifi_init_sta(void);