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
    bmpImage *bmp = malloc(sizeof(bmpImage));
    InitFont(fp,"/spiffs/ziku.bin",16,16);
    bmp_Init(bmp,"/spiffs/epd/2.bmp");

    uint8_t *BlackImage;
    uint16_t Imagesize = ((EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1)) * EPD_HEIGHT;
    if ((BlackImage = (uint8_t *)malloc(Imagesize)) == NULL)
    {
        ESP_LOGI(TAG, "Failed to apply for black memory...");
        return;
    }
    Paint_NewImage(BlackImage, EPD_WIDTH, EPD_HEIGHT, ROTATE_90, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    char *str = "�ú����в�abc�԰��۷�123";
    Paint_DrawString(0,0,fp,str,BLACK,WHITE);
    Paint_DrawString(0,20,fp,str,WHITE,BLACK);
    EPD_Display(BlackImage);
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    Paint_DrawBMP(bmp,BLACK,false);
    EPD_Display(BlackImage);
    ESP_LOGI(TAG, "Goto Sleep.....");
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