#include <stdlib.h>
#include <math.h>
#include "oledDraw.h"

TaskHandle_t Drawtask_handle = NULL;
uint8_t *Drawbuffer = NULL;
PencilColor_t PencilColor = WHITE;
#define CHECKNULL (Drawbuffer == NULL && Drawtask_handle == NULL)
/**
 * @brief FreeRTOS任务 主要是刷新oled屏幕
*/
void DrawTask_flush()
{
    for(;;)
    {
        OLED_Display(Drawbuffer);
        vTaskDelay(FLUSHDELAY/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
/**
 * @brief OLED画板初始化
 * @note  申请画板缓存器内存   创建刷新任务
*/
void OLEDDraw_Init()
{
    if(CHECKNULL)
    {
        Drawbuffer = (uint8_t*) malloc(sizeof(uint8_t) * OLED_DBSIZE);
        xTaskCreate(DrawTask_flush,"DrawTask",1024,NULL,1,&Drawtask_handle);
        OLEDDraw_Clear();
    }
}
/**
 * @brief OLED画板删除
 * @note  释放画板缓存器内存   删除刷新任务
*/
void OLEDDraw_Delete()
{
    if(Drawbuffer) free(Drawbuffer);
    if(Drawtask_handle) vTaskDelete(Drawtask_handle);
}
/**
 * @brief OLED画板清屏幕
*/
void OLEDDraw_Clear()
{
    if(CHECKNULL) return;
    for(int i = 0;i<OLED_DBSIZE;i++)
    Drawbuffer[i] = 0;
}
/**
 * @brief OLED 开启水平滚动
 * @param PageStart  滚动的起始页[0-7]
 * @param PageEnd    滚动的结束页[0-7] 要大于起始页
 * @param ScollFrame 滚动步长时间间隔 帧为单位
 * @param ScollDir   滚动方向 0 向右 1向左
 * @note 调用该函数会挂起刷新屏幕的任务
*/
void OLEDDraw_rowScoll(uint8_t PageStart,uint8_t PageEnd,uint8_t ScollFrame,uint8_t ScollDir)
{
    OLED_Write_Cmd(0X2E);
    OLED_Write_Cmd(ScollDir?0X2A:0x29);
    OLED_Write_Cmd(0X00);
    OLED_Write_Cmd(PageStart&0x07);
    OLED_Write_Cmd(ScollFrame&0x07);
    OLED_Write_Cmd(PageEnd&0x07);
    OLED_Write_Cmd(0X00);
    OLED_Write_Cmd(0XFF);
    vTaskSuspend(Drawtask_handle);
    OLED_Write_Cmd(0X2F);
}
/**
 * @brief OLED 关闭水平滚动
*/
void OLEDDraw_rowScollClose()
{
    OLED_Write_Cmd(0X2E);
    vTaskResume(Drawtask_handle);
}
/**
 * @brief       设置OLED画笔颜色
 * @param color [WHITE BLACK INVERT]
 * @note WHITE 该点点亮 BLACK 该点熄灭 INVERT 该点取反
*/
void OLEDDraw_SetPcolor(PencilColor_t color)
{
    PencilColor = color;
}
/**
 * @brief       OLED画点
 * @param x     横坐标 [0-127]
 * @param y     纵坐标 [0-63]
 * @note WHITE 该点点亮 BLACK 该点熄灭 INVERT 该点取反
*/
void OLEDDraw_point(uint8_t x,uint8_t y)
{
    if(CHECKNULL) return;
    if(!(x<OLED_WIDTH&&y<OLED_HEIGHT)) return;
    switch(PencilColor)
    {
        case WHITE: Drawbuffer[x+(y/8)*OLED_WIDTH] |= 1 << (y%8);       break;
        case BLACK: Drawbuffer[x+(y/8)*OLED_WIDTH] &= ~(1 << (y%8));    break;
        case INVERT:Drawbuffer[x+(y/8)*OLED_WIDTH] ^= 1 << (y%8);       break;
    }
}
/**
 * @brief       OLED画线
 * @param x0    起始点横坐标
 * @param y0    起始点纵坐标
 * @param x1    结束点横坐标
 * @param y1    结束点纵坐标
*/
void OLEDDraw_line(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1)
{
    /* 使用坐标系: 
	                                 127 
	 (0,0)x -------------------------→
			|						|
			|						|
			|						|
			|						|
	        ------------------------- ↓y
			63
*/
    uint8_t ax = abs(x1 - x0);
    uint8_t ay = abs(y1 - y0);
    if(ax == 0 && ay == 0)
        OLEDDraw_point(x0,y0);
    else if(ax == 0 && ay != 0)
    {
        if(y0 > y1) swap_uint8_t(y0,y1);
        if(x1 > OLED_WIDTH) x1 = OLED_WIDTH;
        if(y1 > OLED_HEIGHT) y1 = OLED_HEIGHT;
        if(x0 > OLED_WIDTH || y0 > OLED_HEIGHT) return;
        for(;y0<=y1;y0++) OLEDDraw_point(x0,y0);
    }
    else if(ax != 0 && ay == 0)
    {
        if(x0 > x1) swap_uint8_t(x0,x1);
        if(x1 > OLED_WIDTH) x1 = OLED_WIDTH;
        if(y1 > OLED_HEIGHT) y1 = OLED_HEIGHT;
        if(x0 > OLED_WIDTH || y0 > OLED_HEIGHT) return;
        for(;x0<=x1;x0++) OLEDDraw_point(x0,y0);
    }
    else
    {
        bool NoE = false;
        if(ay > ax)
        {
            swap_uint8_t(x0,y0);
            swap_uint8_t(x1,y1);
        }
        else if(ay == ax) NoE = true; 
        if(x0 > x1) 
        {
            swap_uint8_t(x0,x1);
            swap_uint8_t(y0,y1);
        }   
        if(x1 > OLED_WIDTH) x1 = OLED_WIDTH;
        if(y1 > OLED_HEIGHT) y1 = OLED_HEIGHT;
        if(x0 > OLED_WIDTH || y0 > OLED_HEIGHT) return;
        int8_t e = x0 - x1; 
        int8_t ystep = 1;
        uint8_t dx2 = (x1 - x0) << 2;
        uint8_t dy2 = (y1 - y0) << 2;
        if(y0 > y1) ystep = -1; 
        for(;x0<=x1;x0++)
        {
            OLEDDraw_point(x0,y0);
            if(NoE) y0+=ystep;
            else
            {
                e += dy2;
                if(e > 0)
                {
                    y0+=ystep;
                    e -= dx2;
                }
            }
        }
    }
}
/**
 * @brief           OLED画长方形边框
 * @param x         长方形起始点横坐标[0-127]
 * @param y         长方形起始点纵坐标[0-63]
 * @param w         图片宽[0-127]
 * @param h         图片高[0-63]
*/
void OLEDDraw_rect(uint8_t x,uint8_t y,uint8_t w,uint8_t h)
{
    OLEDDraw_line(x,y,x+w,y);
    OLEDDraw_line(x,y,x,y+h);
    OLEDDraw_line(x+w,y,x+w,y+h);
    OLEDDraw_line(x,y+h,x+w,y+h);
}
/**
 * @brief           OLED画长方形填充
 * @param x         长方形起始点横坐标[0-127]
 * @param y         长方形起始点纵坐标[0-63]
 * @param w         图片宽[0-127]
 * @param h         图片高[0-63]
*/
void OLEDDraw_Fillrect(uint8_t x,uint8_t y,uint8_t w,uint8_t h)
{
    for(uint8_t x0=x;x0<x+w;x0++)
        OLEDDraw_line(x0,y,x0,y+h);
}
/**
 * @brief           OLED画圆
 * @param x         圆心横坐标[0-127]
 * @param y         圆心纵坐标[0-63]
 * @param r         圆心半径
*/
void OLEDDraw_circle(uint8_t x,uint8_t y,uint8_t r)
{
    uint8_t x0 = 1,y0 = r;
    int8_t dp = 1 - r;
    for(;x0<y0;x0++)
    {
        if(dp < 0) dp += x0*2+2;
        else dp += x0*2-(y0--)*2+6; 
        OLEDDraw_point(x+x0,y+y0);
        OLEDDraw_point(x+x0,y-y0);
        OLEDDraw_point(x-x0,y+y0);
        OLEDDraw_point(x-x0,y-y0);
        OLEDDraw_point(x+y0,y+x0);
        OLEDDraw_point(x+y0,y-x0);
        OLEDDraw_point(x-y0,y+x0);
        OLEDDraw_point(x-y0,y-x0);
    }
    OLEDDraw_point(x+r,y);
    OLEDDraw_point(x,y+r);
    OLEDDraw_point(x-r,y);
    OLEDDraw_point(x,y-r);
}
/**
 * @brief           OLED画圆部分
 * @param x         圆心横坐标[0-127]
 * @param y         圆心纵坐标[0-63]
 * @param r         圆心半径
 * @param quads     选择画圆边界
*/
void OLEDDraw_circleQuads(uint8_t x,uint8_t y,uint8_t r,uint8_t quads)
{
    uint8_t x0 = 1,y0 = r;
    int8_t dp = 1 - r;
    for(;x0<y0;x0++)
    {
        if(dp < 0) dp += x0*2+2;
        else dp += x0*2-(y0--)*2+6; 
        if (quads & 0x1) {
            OLEDDraw_point(x+x0,y-y0);
            OLEDDraw_point(x+y0,y-x0);
        }
        if (quads & 0x2) {
            OLEDDraw_point(x-x0,y-y0);
            OLEDDraw_point(x-y0,y-x0);
        }
        if (quads & 0x4) {
            OLEDDraw_point(x-x0,y+y0);
            OLEDDraw_point(x-y0,y+x0);
        }
        if (quads & 0x8) {
            OLEDDraw_point(x+x0,y+y0);
            OLEDDraw_point(x+y0,y+x0);
        }
    }
    if (quads & 0x1 && quads & 0x8) OLEDDraw_point(x+r,y);
    if (quads & 0x4 && quads & 0x8) OLEDDraw_point(x,y+r);
    if (quads & 0x2 && quads & 0x4) OLEDDraw_point(x-r,y);
    if (quads & 0x1 && quads & 0x2) OLEDDraw_point(x,y-r);
}
/**
 * @brief           OLED填充圆
 * @param x         圆心横坐标[0-127]
 * @param y         圆心纵坐标[0-63]
 * @param r         圆心半径
*/
void OLEDDraw_Fillcircle(uint8_t x,uint8_t y,uint8_t r)
{
    uint8_t x0 = 1,y0 = r;
    int8_t dp = 1 - r;
    for(;x0<y0;x0++)
    {
        if(dp < 0) dp += x0*2+2;
        else dp += x0*2-(y0--)*2+6; 
        OLEDDraw_line(x+x0,y+y0,x-x0,y+y0);
        OLEDDraw_line(x+y0,y+x0,x-y0,y+x0);
        OLEDDraw_line(x+x0,y-y0,x-x0,y-y0);
        OLEDDraw_line(x+y0,y-x0,x-y0,y-x0);
    }
    OLEDDraw_line(x+r,y,x-r,y);
}
/**
 * @brief           OLED画进度条
 * @param x         圆心横坐标[0-127]
 * @param y         圆心纵坐标[0-63]
 * @param r         圆心半径
*/
void OLEDDraw_progressBar(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t progress)
{
    if(w < 2*h) return;
    uint8_t radius = h / 2;
    uint8_t xRadius = x+radius;
    uint8_t yRadius = y+radius;
    uint8_t innerRadius = radius - 2;
    OLEDDraw_SetPcolor(WHITE);
    OLEDDraw_circleQuads(xRadius,yRadius,radius,0b00000110);
    OLEDDraw_line(xRadius,y,x+w-radius,y);
    OLEDDraw_line(xRadius,y+h,x+w-radius,y+h);
    OLEDDraw_circleQuads(x+w-radius,yRadius,radius,0b00001001);

    uint8_t maxProgressWidth = (w-h+1)*progress/100;
    OLEDDraw_Fillcircle(xRadius,yRadius,innerRadius);
    OLEDDraw_Fillrect(xRadius+1,y+2,maxProgressWidth,h-3);
    OLEDDraw_Fillcircle(xRadius+maxProgressWidth,yRadius,innerRadius);

}
/**
 * @brief           OLED画一张图片
 * @param x         起始点横坐标[0-127]
 * @param y         起始页纵坐标[0-7]
 * @param w         图片宽[0-128]
 * @param h         图片高[0-8]*8
 * @param buffer    图片数组指针
*/
void OLEDDraw_buffer(uint8_t x,uint8_t y,uint8_t w, uint8_t h, uint8_t* buffer)
{
    uint8_t i,j;
    for(i=0;i<h;i++)
    {
        for(j=0;j<w;j++)
            Drawbuffer[(x+j)+(y+i)*OLED_WIDTH] = buffer[j+i*w];
    }
}
