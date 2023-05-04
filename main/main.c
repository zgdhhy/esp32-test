#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"

#include "myspiffs.h"
#include "epaper.h"

static const char *TAG = "EPD";

void epd_test()
{
    EPD_ALL_Init();
    FontFile *fp = malloc(sizeof(FontFile));
//    bmpImage *bmp = malloc(sizeof(bmpImage));
    InitFont(fp,"/spiffs/ziku.bin",16,16);
//    bmp_Init(bmp,"/spiffs/epd/2.bmp");

    uint8_t *BlackImage;
    uint8_t *BlackImage1;
//    uint16_t Imagesize = ((EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1)) * EPD_HEIGHT;
    if ((BlackImage = (uint8_t *)malloc(16*16)) == NULL)
    {
        ESP_LOGI(TAG, "Failed to apply for black memory...");
        return;
    }
    if ((BlackImage1 = (uint8_t *)malloc(16*16)) == NULL)
    {
        ESP_LOGI(TAG, "Failed to apply for black memory...");
        return;
    }
    Paint_NewImage(BlackImage, 16, 16, ROTATE_0, WHITE);
    Paint_NewImage(BlackImage1, 16, 16, ROTATE_0, WHITE);

    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawChar(0,0,fp,"ÕÔ",BLACK,WHITE);
    EPD_Partial_Display(0,0,BlackImage,16,16);
    Paint_SelectImage(BlackImage1);
    Paint_Clear(WHITE);
    Paint_DrawChar(0,0,fp,"ÕÔ",BLACK,WHITE);
    EPD_Partial_Display(64,64,BlackImage,16,16);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "1");

    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawChar(0,0,fp,"¹ú",BLACK,WHITE);
    EPD_Partial_Display(0,0,BlackImage,16,16);
    Paint_SelectImage(BlackImage1);
    Paint_Clear(WHITE);
    Paint_DrawChar(0,0,fp,"¹ú",BLACK,WHITE);
    EPD_Partial_Display(64,64,BlackImage,16,16);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "2");

    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawChar(0,0,fp,"¶°",BLACK,WHITE);
    EPD_Partial_Display(0,0,BlackImage,16,16);
    Paint_SelectImage(BlackImage1);
    Paint_Clear(WHITE);
    Paint_DrawChar(0,0,fp,"¶°",BLACK,WHITE);
    EPD_Partial_Display(64,64,BlackImage,16,16);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "3");

    EPD_Sleep();
    free(BlackImage);
    BlackImage = NULL;
    CloseFont(fp);
    return;
}

void SPI_test_task()
{
    epd_test();
    for (;;)
    {
        vTaskDelay(100);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    spiffs_init(5);
    xTaskCreate(SPI_test_task, "SPI_test", 2048, NULL, 3, NULL);
}