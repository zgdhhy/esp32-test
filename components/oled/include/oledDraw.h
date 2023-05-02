#ifndef OLEDDRAW_H
#define OLEDDRAW_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_DBSIZE     OLED_WIDTH * OLED_HEIGHT / 8  

#define FLUSHDELAY      20     //OLED刷新周期 ms
#define swap_uint8_t(a, b) { uint8_t t = a; a = b; b = t; }
typedef enum
{
    WHITE = 0,
    BLACK,
    INVERT
} PencilColor_t;

void OLEDDraw_Init();
void OLEDDraw_Delete();
void OLEDDraw_Clear();
void OLEDDraw_rowScoll(uint8_t PageStart,uint8_t PageEnd,uint8_t ScollFrame,uint8_t ScollDir);
void OLEDDraw_rowScollClose();
void OLEDDraw_SetPcolor(PencilColor_t color);
void OLEDDraw_point(uint8_t x,uint8_t y);
void OLEDDraw_line(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1);
void OLEDDraw_rect(uint8_t x,uint8_t y,uint8_t w,uint8_t h);
void OLEDDraw_Fillrect(uint8_t x,uint8_t y,uint8_t w,uint8_t h);
void OLEDDraw_circle(uint8_t x,uint8_t y,uint8_t r);
void OLEDDraw_circleQuads(uint8_t x,uint8_t y,uint8_t r,uint8_t quads);
void OLEDDraw_Fillcircle(uint8_t x,uint8_t y,uint8_t r);
void OLEDDraw_progressBar(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t progress);
void OLEDDraw_buffer(uint8_t x,uint8_t y,uint8_t w, uint8_t h, uint8_t* buffer);
#endif