#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define EXAMPLE_ESP_WIFI_SSID      "ESPCar"
#define EXAMPLE_ESP_WIFI_PASS      "vroumpassword"
#define EXAMPLE_MAX_STA_CONN       4

#define PORT 8888

/**
 * MotorOrder represent the received order
 * motorPwm : The speed of the motor 
 * motorDirection : 0 : Forward
 *                  1 : Backward
 * steeringDirection : 0 : Neutral
 *                     1 : Right
 *                     2 : Left
 */
typedef struct MotorOrder{
    int motorPwm;
    int motorDirection;
    int steeringDirection;
} MotorOrder;

void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);


void wifi_init_softap();

void carUDPServerTask(void *pvParameters);

#endif