#ifndef EPAPERHAL_H
#define EPAPERHAL_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "string.h"

/**
 * GPIO config ramap
**/
#define EPD_SCK_PIN  13
#define EPD_MOSI_PIN 14
#define EPD_CS_PIN   15
#define EPD_RST_PIN  26
#define EPD_DC_PIN   27
#define EPD_BUSY_PIN 25

/**
 * 配置spi参数
**/
#define HOST_ID         SPI2_HOST

void EPD_GPIO_Init();
void EPD_SPI_Init();
void EPD_write_bytes(const uint8_t *data, size_t length);
void EPD_write_byte(const uint8_t data);

void EPD_Reset(void);
void EPD_SendCommand(uint8_t comd);
void EPD_SendData(uint8_t data);
void EPD_SendDatas(const uint8_t *data, size_t length);
#endif