#ifndef EDRIVER_H
#define EDRIVER_H

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
 * 设置墨水屏尺寸
**/
#define EPD_WIDTH   128
#define EPD_HEIGHT  296
/**
 * 配置spi参数
**/
#define HOST_ID         SPI2_HOST


void EPD_GPIO_Init();
void EPD_SPI_Init();
void EPD_write_bytes(const uint8_t *data,size_t length);
void EPD_write_byte(const uint8_t data);
void EPD_Reset(void);
void EPD_SendCommand(uint8_t comd);
void EPD_SendData(uint8_t data);
void EPD_SendDatas(const uint8_t *data,size_t length);
void EPD_ReadBusy(void);
void EPD_Init(void);
void EPD_Clear(void);
void EPD_Display(const uint8_t *blackimage);
void EPD_Partial_Display(uint16_t x, uint16_t y,const uint8_t *image,uint16_t PART_COLUMN, uint16_t PART_PAGE);
void EPD_Sleep(void);
void EPD_ALL_Init(void);
#endif
