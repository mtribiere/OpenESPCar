/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"
#include "driver/pwm.h"
#include "driver/gpio.h"

#include "ConnectionManager.h"

#define PWM_PERIOD    (1000)
#define MOTOR_PIN 12
#define DIRECTION_LEFT_PIN 16
#define DIRECTION_RIGHT_PIN 5
#define FORWARD_PIN 4
#define BACKWARD_PIN 0
#define GPIO_OUTPUT_SEL (1ULL << DIRECTION_LEFT_PIN) | (1ULL << DIRECTION_RIGHT_PIN) | (1ULL << FORWARD_PIN) | (1ULL << BACKWARD_PIN)


static const char *TAG = "ESPCar";

int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void motorControlTask(void *pvParameters){

    //Receive parameters
    MotorOrder *sharedOrder = (struct MotorOrder*) pvParameters;

    //Configure output pin
    gpio_config_t io_config;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = GPIO_OUTPUT_SEL;
    io_config.pull_down_en = 0;
    io_config.pull_up_en = 0;
    gpio_config(&io_config);
    
    //Configure PWM pins
    uint32_t pwmPin[] = {MOTOR_PIN};
    uint32_t duties[] = {0};
    float phase[] = {0};
    
    pwm_init(PWM_PERIOD,duties,1,pwmPin);
    pwm_set_phases(phase);
    pwm_start();

    while(1){
        
        //Set steeering pin
        gpio_set_level(FORWARD_PIN,(sharedOrder->motorDirection ) == 0x00 ? 1 : 0);
        gpio_set_level(BACKWARD_PIN,(sharedOrder->motorDirection) == 0x01 ? 1 : 0);

    
        //Set the pwm value
        pwm_set_duty(0, map(sharedOrder->motorPwm,0,100,0,PWM_PERIOD));
        pwm_start();
        vTaskDelay(50 / portTICK_RATE_MS);

        //Set steeering pin
        gpio_set_level(DIRECTION_RIGHT_PIN,(sharedOrder->steeringDirection) == 0x01 ? 1 : 0);
        gpio_set_level(DIRECTION_LEFT_PIN,(sharedOrder->steeringDirection) == 0x02 ? 1 : 0);
    
    }

    
}


void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //Create shared motor command struct
    MotorOrder motorOrder;

    //Create Wifi point
    ESP_LOGI(TAG, "========= ESPCar : Creating wifi point");
    wifi_init_softap();

    //UDP Socket task
    ESP_LOGI(TAG, "========= ESPCar : Creating UDP server task");
    xTaskCreate(carUDPServerTask, "carUDPServer", 4096, &motorOrder, 5, NULL);

    //Motor control task
    ESP_LOGI(TAG, "========= ESPCar : Creating motor control task");
    xTaskCreate(motorControlTask, "motorControl", 1024, &motorOrder, 8, NULL);

}
