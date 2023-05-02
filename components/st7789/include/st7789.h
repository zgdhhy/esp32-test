#ifndef ST7789_H
#define ST7789_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "string.h"
#include "math.h"
#include "font.h"

#define HOST_ID         SPI2_HOST 
#define SPI_CMD_MODE    0 
#define SPI_DATA_MODE   1  

#define RED			    0xf800
#define GREEN			0x07e0
#define BLUE			0x001f
#define BLACK			0x0000
#define WHITE			0xffff
#define GRAY			0x8c51
#define YELLOW			0xFFE0
#define CYAN			0x07FF
#define PURPLE			0xF81F


typedef struct {
    uint16_t _width;                //设备的宽
	uint16_t _height;               //设备的高
	uint16_t _offsetx;              //设备的x偏移
	uint16_t _offsety;              //设备的y偏移
	uint16_t _font_fill;            //设备填充是否打开
	uint16_t _font_fill_color;      //设备的填充颜色
	uint16_t _font_underline;       //设备的下划线是否打开
	uint16_t _font_underline_color; //设备的下划线颜色
    int8_t _dc;                     //设备的dc引脚 0数据 1命令
    int8_t _blk;                    //设备的blk引脚
    spi_device_handle_t _SPIHandle; //spi设备的句柄
}TFT_t;

void spi_st7789_init(TFT_t *tft,int8_t MOSI_GPIO,int8_t SCLK_GPIO,int8_t DC_GPIO,int8_t CS_GPIO,int8_t RES_GPIO, int8_t BLK_GPIO);
void spi_st7789_write_byte(spi_device_handle_t handle,const uint8_t *data,size_t length);
void spi_st7789_write_command(TFT_t *tft,uint8_t cmd);
void spi_st7789_write_data(TFT_t *tft,uint8_t data);
void spi_st7789_write_addr(TFT_t *tft, uint16_t addr1, uint16_t addr2);
void spi_st7789_write_color(TFT_t *tft, uint16_t color, uint16_t size);
void spi_st7789_write_colors(TFT_t *tft, uint16_t *colors, uint16_t size);

void ST7789_Init(TFT_t * tft, int width, int height, int offsetx, int offsety);
void ST7789_Drawpoint(TFT_t * tft, uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawMultipoint(TFT_t * tft, uint16_t x,uint16_t y,uint16_t size,uint16_t *colors);
void ST7789_DrawLine(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ST7789_DrawRect(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ST7789_DrawFillRect(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ST7789_DrawRectAngle(TFT_t *tft, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color);
void ST7789_DrawTriangle(TFT_t *tft, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color);
void ST7789_DrawCircle(TFT_t *tft, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ST7789_DrawFillCircle(TFT_t *tft, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ST7789_DrawRoundRect(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t color);
void ST7789_DrawArrow(TFT_t *tft, uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t w,uint16_t color);
void ST7789_DrawFillArrow(TFT_t *tft, uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t w,uint16_t color);
int ST7789_DrawChar(TFT_t *tft, FontFile *fp, char* font, uint8_t x, uint8_t y, uint16_t color);
int ST7789_DrawString(TFT_t *tft, FontFile *fp, char* fonts, uint16_t x, uint16_t y, uint16_t color);
void ST7789_DisplayOff(TFT_t *tft);
void ST7789_DisplayOn(TFT_t *tft);
void ST7789_FillScreen(TFT_t *tft, uint16_t color);
void ST7789_SetFontDirection(TFT_t *tft, uint16_t dir);
void ST7789_SetFontFill(TFT_t *tft, uint16_t color);
void ST7789_UnsetFontFill(TFT_t *tft);
void ST7789_SetFontUnderLine(TFT_t *tft, uint16_t color);
void ST7789_UnsetFontUnderLine(TFT_t *tft);
void ST7789_BacklightOn(TFT_t *tft);
void ST7789_BacklightOff(TFT_t *tft);
void ST7789_InversionOn(TFT_t *tft);
void ST7789_InversionOff(TFT_t *tft);
uint16_t rgb565_conv(uint16_t r,uint16_t g,uint16_t b);
#endif