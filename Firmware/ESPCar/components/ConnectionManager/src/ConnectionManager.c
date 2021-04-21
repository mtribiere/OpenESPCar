#include "ConnectionManager.h"
static const char *TAG = "ESPCar";

void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap()
{
    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void carUDPServerTask(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    char isConnected = 0;
    int err;

    MotorOrder *sharedOrder = (MotorOrder*) pvParameters;
    
    //Creating and binding socket
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket binded");

    ////////////////////////////////////////////////CONNECTION ROUTINE///////////////////////////////////////////////////
    struct sockaddr_in sourceAddr;
    socklen_t socklen = sizeof(sourceAddr);

    while(!isConnected){
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

        // Error occured during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        }
        // Data received
        else {
            
            //Check if this is a connection request
            if(rx_buffer[0] == 0x01 && rx_buffer[1] == 'H' && rx_buffer[2] == 'E' && rx_buffer[3] == 'L' && rx_buffer[4] == 'L' && rx_buffer[5] == 'O'){
                char helloResponse[] = {0x02,'O','K'};

                err = sendto(sock, helloResponse, 3, 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                isConnected = true;

                ESP_LOGI(TAG, "========= ESPCar : Connection successfull");
            }
            
        }

        if (err < 0) {
            ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
            isConnected = false;
            break;
        }
    }

    ///////////////////////////////////////////////MAIN LOOP/////////////////////////////////////////////////////////////
    //While connection is alive
    while (isConnected) {

        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

        // Error occured during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        }
        // Data received
        else {

            //////DEBUG
            for(int i = 0;i<len;i++)
                printf("%d ",rx_buffer[i]);
            printf("\n");

            //Fill up the shared order structure
            sharedOrder->motorPwm = rx_buffer[1];
            sharedOrder->motorDirection = rx_buffer[2];
            sharedOrder->steeringDirection = rx_buffer[3];
            
            if (err < 0) {
                ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                break;
            }
        }
    }

    if(err){
        ESP_LOGE(TAG, "Error, shutting down socket and restarting...");
        if (sock != -1) {
            shutdown(sock, 0);
            close(sock);
        }
    }

    //Reboot rooting
    ESP_LOGI(TAG, "Going down for reboot !");
    esp_restart();

    vTaskDelete(NULL);
}
