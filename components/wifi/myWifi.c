#include "myWifi.h"

//wifi连接成功信号量
SemaphoreHandle_t wificonnect;
//wifi是否连接成功
bool WifiIsconnect;

static void wifi_event_handler(void* arg,esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(WIFI_TAG,"WIFI 开始工作");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(WIFI_TAG,"WIFI 已连接");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(WIFI_TAG,"WIFI 断开连接");
                WifiIsconnect = false;
                break;
            default:break;
        }
    }
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        WifiIsconnect = true;
        xSemaphoreGive(wificonnect);
        ip_event_got_ip_t *IP_MSG = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(WIFI_TAG,"已获取IP地址" IPSTR,IP2STR(&IP_MSG->ip_info.ip));
    }
}

void wifi_sta_handler_init()
{
    wificonnect = xSemaphoreCreateBinary();
    esp_netif_create_default_wifi_sta();
    esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event_handler,NULL,NULL);
    esp_event_handler_instance_register(IP_EVENT,ESP_EVENT_ANY_ID,wifi_event_handler,NULL,NULL);
    // wifi初始化
    wifi_init_config_t conf = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&conf);
    wifi_config_t sta_config = {
        .sta = {
            .ssid = ESP_STA_WIFI_SSID,
            .password = ESP_STA_WIFI_PASS,
        }
    };
    // 设置工作模式
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA,&sta_config);
    // 启动wifi
    esp_wifi_start();
    ESP_LOGI(WIFI_TAG,"wifi_sta_init finished");
    xSemaphoreTake(wificonnect,portMAX_DELAY);;
} 

void wifi_sta_init(esp_event_handler_t event_handler)
{
    esp_netif_create_default_wifi_sta();
    esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,event_handler,NULL,NULL);
    esp_event_handler_instance_register(IP_EVENT,ESP_EVENT_ANY_ID,event_handler,NULL,NULL);
    // wifi初始化
    wifi_init_config_t conf = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&conf);
    wifi_config_t sta_config = {
        .sta = {
            .ssid = ESP_STA_WIFI_SSID,
            .password = ESP_STA_WIFI_PASS,
        }
    };
    // 设置工作模式
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA,&sta_config);
    // 启动wifi
    esp_wifi_start();
    ESP_LOGI(WIFI_TAG,"wifi_sta_init finished");
} 