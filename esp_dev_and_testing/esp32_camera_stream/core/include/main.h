#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define pdSECOND pdMS_TO_TICKS(5000)
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include <esp_http_server.h>
#include "Wifi.h"
#include <esp_camera.h>
#include "camera1.h"
#include "Server1.h"
