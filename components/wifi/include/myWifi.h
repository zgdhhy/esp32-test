#ifndef MYWIFI_H
#define MYWIFI_H

#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#define ESP_STA_WIFI_SSID    "zzzzdddd"
#define ESP_STA_WIFI_PASS    "12345678z"
#define WIFI_TAG             "WIFI"

extern bool WifiIsconnect;
void wifi_sta_handler_init();
void wifi_sta_init(esp_event_handler_t event_handler);
#endif