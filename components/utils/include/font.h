#ifndef FONT_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
typedef struct {
    const char* path;   // 文件路径
    uint8_t wight;     // 文件字体宽
    uint8_t height;     // 文件字体高
    uint16_t GBKoffset; // GBK字符的偏移量
    bool isopen;        // 文件是否打开
    FILE *file;         // 文件指针
} FontFile;

typedef struct {
    uint8_t *code;      // 字体缓存
    uint8_t wight;     // 字体宽
    uint8_t height;     // 字体高
    uint8_t size;       // 字体缓存尺寸
} FontCode;

void InitFont(FontFile *fp, const char* path, uint8_t wight, uint8_t height);
bool OpenFont(FontFile *fp);
void CloseFont(FontFile* fp);
bool GetFontCode(FontFile *fp,char *font,FontCode *fcode);
bool GetGBKCode(FontFile *fp,char *font,FontCode *fcode);
bool GetASCIICode(FontFile *fp,char *font,FontCode *fcode);
void AddDrowline(FontCode *fcode);
void InvertFont(FontCode *fcode);
void ShowFont(FontCode *fcode);
#endif