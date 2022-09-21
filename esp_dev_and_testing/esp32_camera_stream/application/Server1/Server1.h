#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "camera1.h"

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

#define TAG "Server1"

httpd_handle_t start_webserver(void);

void stop_webserver(httpd_handle_t server);
void connect_handler(void *arg, esp_event_base_t event_base,
                     int32_t event_id, void *event_data);
void disconnect_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);

