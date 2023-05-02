#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
// 加载数据到缓冲区
#define UINT8_2_BUFFER(p, u8)       {*(p++) = (uint8_t)u8;}
#define UINT16_2_BUFFER(p, u16)     {*(p++) = (uint8_t)u16;*(p++) = (uint8_t)(u16>>8);}
#define ARRAY_2_BUFFER(p, a, len)   {for(int ijk=0;ijk<len;ijk++) *(p++) = (uint8_t)a[ijk];}
#endif
