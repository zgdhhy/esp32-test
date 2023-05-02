#include "myOTA.h"

static const char *TAG = "MyOTA";
esp_err_t ota_http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) 
    {
        case HTTP_EVENT_ERROR:              //错误事件
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:       //连接成功事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:        //发送头事件
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:          //接收头事件
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:            //接收数据事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:          //会话完成事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:       //断开事件
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}
void ota_upgrade(char* url)
{
    ESP_LOGI(TAG, "OTA Starting");
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = ota_http_event_handler,
        .keep_alive_enable = true,
    };

    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
}

void ota_upgrade_progress(char* url,ota_progress_cb ota_progress_handler)
{
    ESP_LOGI(TAG, "OTA Starting");
    esp_http_client_config_t config = {
        .url = url,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    esp_https_ota_handle_t ota_handle = NULL;

    esp_err_t err = esp_https_ota_begin(&ota_config, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        vTaskDelete(NULL);
    }
    int all_size = esp_https_ota_get_image_size(ota_handle);
    while (1) {
        err = esp_https_ota_perform(ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) break;
        if (ota_progress_handler) ota_progress_handler(esp_https_ota_get_image_len_read(ota_handle),all_size);
    }

    if (esp_https_ota_is_complete_data_received(ota_handle) != true)
        ESP_LOGE(TAG, "Complete data was not received.");
    else 
    {
        esp_err_t ota_finish_err = esp_https_ota_finish(ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) 
        {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart();
        } 
        else 
        {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED)
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
            vTaskDelete(NULL);
        }
    }
    esp_https_ota_abort(ota_handle);
    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
    vTaskDelete(NULL);
}