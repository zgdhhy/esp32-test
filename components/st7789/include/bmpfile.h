#ifndef BMPFILE_H
#define BMPFILE_H
#include <stdint.h>
#include "st7789.h"
typedef struct {
    uint8_t bhtype[2];      /* 文件类型 BMP格式的文件这两个字节是0x4D42 "BM"*/
    uint32_t bhsize;        /* 文件大小 */
    uint16_t reserved1;     /* 保留，必须设置为0 */
    uint16_t reserved2;     /* 保留，必须设置为0 */
    uint32_t offset;        /* 从头到位图数据的偏移 */
} bmp_header_t;

typedef struct {
    uint32_t bisize;        /* 信息头的大小 (40 bytes) */
    uint32_t biwidth;       /* 以像素为单位说明图像的宽度 */
    uint32_t biheight;      /* 以像素为单位说明图像的高度 */
    uint16_t biplanes;      /* 为目标设备说明颜色平面数，总被设置为1 */
    uint16_t biBitCount;    /* 说明比特数/像素数，值有1、2、4、8、16、24、32 */
    uint32_t biCompression; /* 说明图像的压缩类型，最常用的就是0（BI_RGB），表示不压缩*/
    uint32_t biSizeImages;  /* 说明位图数据的大小，当用BI_RGB格式时，可以设置为0 */
    uint32_t biXPels;       /* 表示水平分辨率，单位是像素/米，有符号整数 */
    uint32_t biYPels;       /* 表示垂直分辨率，单位是像素/米，有符号整数 */
    uint32_t biClrUsed;     /* 说明位图使用的调色板中的颜色索引数，为0说明使用所有 */
    uint32_t biClrImportant;/* 说明对图像显示有重要影响的颜色索引数，为0说明都重要 */
} bmp_info_t;

typedef struct {
    bmp_header_t header;
    bmp_info_t   info;
} bmpfile_t;

void bmp_show(TFT_t *tft,char* file,uint16_t width,uint16_t height);
#endif

