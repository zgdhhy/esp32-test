#ifndef DISPLAY_H
#define DISPLAY_H
#include "st7789.h"

typedef struct {
    uint16_t _x;                    //窗口的开始横坐标
    uint16_t _y;                    //窗口的开始纵坐标
    uint16_t _w;                    //窗口的宽
	uint16_t _h;                    //窗口的高
    uint16_t _font_fill;            //窗口填充是否打开
	uint16_t _font_fill_color;      //窗口的填充颜色
	uint16_t _font_underline;       //窗口的下划线是否打开
	uint16_t _font_underline_color; //窗口的下划线颜色
    TFT_t *tft;
}display_window_t;

display_window_t* Display_Init(TFT_t* tft,uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void Display_Drawpoint(display_window_t* window, uint16_t x, uint16_t y, uint16_t color);
void Display_DrawMultipoint(display_window_t* window, uint16_t x,uint16_t y,uint16_t size,uint16_t *colors);
void Display_DrawLine(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void Display_DrawRect(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void Display_DrawFillRect(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void Display_DrawRectAngle(display_window_t* window, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color);
void Display_DrawTriangle(display_window_t* window, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color);
void Display_DrawCircle(display_window_t* window, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void Display_DrawFillCircle(display_window_t* window, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void Display_DrawRoundRect(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t color);
void Display_DrawArrow(display_window_t* window, uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t w,uint16_t color);
void Display_DrawFillArrow(display_window_t* window, uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t w,uint16_t color);
int Display_DrawChar(display_window_t* window, FontFile *fp, char* font, uint8_t x, uint8_t y, uint16_t color);
int Display_DrawString(display_window_t* window, FontFile *fp, char* fonts, uint16_t x, uint16_t y, uint16_t color);
void Display_FillScreen(display_window_t* window, uint16_t color);
void Display_SetFontDirection(display_window_t* window, uint16_t dir);
void Display_SetFontFill(display_window_t* window, uint16_t color);
void Display_UnsetFontFill(display_window_t* window);
void Display_SetFontUnderLine(display_window_t* window, uint16_t color);
void Display_UnsetFontUnderLine(display_window_t* window);
#endif