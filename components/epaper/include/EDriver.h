#ifndef EDRIVER_H
#define EDRIVER_H

#include "string.h"
#include "EpaperHal.h"

// #define EPD_2IN9
#define EPD_2IN66

#ifdef EPD_2IN9
//墨水屏尺寸
#define EPD_WIDTH   128
#define EPD_HEIGHT  296

void EPD_ReadBusy(void);
void EPD_Init(void);
void EPD_Clear(void);
void EPD_Display(const uint8_t *blackimage);
void EPD_Partial_Display(uint16_t x, uint16_t y, const uint8_t *image, uint16_t PART_COLUMN, uint16_t PART_PAGE);
#endif

#ifdef EPD_2IN66
//墨水屏尺寸
#define EPD_WIDTH   152
#define EPD_HEIGHT  296

void EPD_ReadBusy(void);
void EPD_Init(void);
void EPD_Clear(void);
void EPD_Display(uint8_t *blackimage, uint8_t *redimage);
#endif

void EPD_Sleep(void);
void EPD_ALL_Init(void);
#endif
