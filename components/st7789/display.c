#include "display.h"

/**
 * @brief 窗口初始化
 * @param tft       tft设备
 * @param x         窗口相对于设备的横坐标
 * @param y         窗口相对于设备的纵坐标
 * @param w         窗口的宽
 * @param h         窗口的高
 * @return display_window_t展示窗口指针 NULL 初始化失败 
*/
display_window_t* Display_Init(TFT_t* tft,uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (x+w > tft->_width) return NULL;
	if (y+h > tft->_height) return NULL;
    display_window_t* window = (display_window_t*)malloc(sizeof(display_window_t));
    window->_x = x;
    window->_y= y;
    window->_w = w;
	window->_h = h;
	window->_font_fill = false;
	window->_font_underline = false;
    window->tft = tft;
    return window;
}
/**
 * @brief 画一个点
 * @param window     展示窗口
 * @param x          点的x坐标
 * @param y          点的y坐标
 * @param color      点的颜色
*/
void Display_Drawpoint(display_window_t* window, uint16_t x, uint16_t y, uint16_t color)
{
    if(x > window->_w) return;
    if(y > window->_h) return;
    ST7789_Drawpoint(window->tft,x+window->_x,y+window->_y,color);
}
/**
 * @brief 画一堆点
 * @param window     展示窗口
 * @param x          开始x坐标
 * @param y          开始y坐标
 * @param size       多少个点啊[colors数组的尺寸]
 * @param colors     颜色堆数组
*/
void Display_DrawMultipoint(display_window_t* window, uint16_t x,uint16_t y,uint16_t size,uint16_t *colors)
{
	if (x+size > window->_w) return;
	if (y >= window->_h) return;
    ST7789_DrawMultipoint(window->tft,x+window->_x,y+window->_y,size,colors);
}
/**
 * @brief 画一条线
 * @param window     展示窗口
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param color      线的颜色
*/
void Display_DrawLine(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) 
{
	int i;
	int dx,dy;
	int sx,sy;
	int E;
	/* distance between two points */
	dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
	dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

	/* direction of two point */
	sx = ( x2 > x1 ) ? 1 : -1;
	sy = ( y2 > y1 ) ? 1 : -1;

	/* inclination < 1 */
	if ( dx > dy ) 
    {
		E = -dx;
		for ( i = 0 ; i <= dx ; i++ ) 
        {
			Display_Drawpoint(window, x1, y1, color);
			x1 += sx;
			E += 2 * dy;
			if ( E >= 0 ) 
            {
			y1 += sy;
			E -= 2 * dx;
		    }
	    }
	/* inclination >= 1 */
	} 
    else 
    {
		E = -dy;
		for ( i = 0 ; i <= dy ; i++ ) 
        {
			Display_Drawpoint(window, x1, y1, color);
			y1 += sy;
			E += 2 * dx;
			if ( E >= 0 ) 
            {
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}
/**
 * @brief 画一个矩形
 * @param window     展示窗口
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param color      边框颜色
*/
void Display_DrawRect(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) 
{
	Display_DrawLine(window, x1, y1, x2, y1, color);
	Display_DrawLine(window, x2, y1, x2, y2, color);
	Display_DrawLine(window, x2, y2, x1, y2, color);
	Display_DrawLine(window, x1, y2, x1, y1, color);
}
/**
 * @brief 画一个实心矩形
 * @param window     展示窗口
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param color      填充的颜色
 * @note 对角矩阵
*/
void Display_DrawFillRect(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) 
{
	if (x1 >= window->_w) return;
	if (x2 >= window->_w) x2=window->_w-1;
	if (y1 >= window->_h) return;
	if (y2 >= window->_h) y2=window->_h-1;
    ST7789_DrawFillRect(window->tft,x1+window->_x,y1+window->_y,x2+window->_x,y2+window->_y,color);
}
/**
 * @brief 画一个有角度的矩形
 * @param window     展示窗口
 * @param xc         矩形中心x坐标
 * @param yc         矩形中心y坐标
 * @param w          矩形宽
 * @param h          矩形高
 * @param angle      旋转的角度
 * @param color      填充的颜色
*/
void Display_DrawRectAngle(display_window_t* window, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color)
{
	double xd,yd,rd;
	int x1,y1;
	int x2,y2;
	int x3,y3;
	int x4,y4;
	rd = -angle * M_PI / 180.0;
	xd = 0.0 - w/2;
	yd = h/2;
	x1 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y1 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	yd = 0.0 - yd;
	x2 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y2 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	xd = w/2;
	yd = h/2;
	x3 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y3 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	yd = 0.0 - yd;
	x4 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y4 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	Display_DrawLine(window, x1, y1, x2, y2, color);
	Display_DrawLine(window, x1, y1, x3, y3, color);
	Display_DrawLine(window, x2, y2, x4, y4, color);
	Display_DrawLine(window, x3, y3, x4, y4, color);
}
/**
 * @brief 画一个有角度的三角形
 * @param window     展示窗口
 * @param xc         三角形中心x坐标
 * @param yc         三角形中心y坐标
 * @param w          三角形底
 * @param h          三角形高
 * @param angle      旋转的角度
 * @param color      边框的颜色
*/
void Display_DrawTriangle(display_window_t* window, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color)
{
	double xd,yd,rd;
	int x1,y1;
	int x2,y2;
	int x3,y3;
	rd = -angle * M_PI / 180.0;
	xd = 0.0;
	yd = h/2;
	x1 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y1 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	xd = w/2;
	yd = 0.0 - yd;
	x2 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y2 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	xd = 0.0 - w/2;
	x3 = (int)(xd * cos(rd) - yd * sin(rd) + xc);
	y3 = (int)(xd * sin(rd) + yd * cos(rd) + yc);

	Display_DrawLine(window, x1, y1, x2, y2, color);
	Display_DrawLine(window, x1, y1, x3, y3, color);
	Display_DrawLine(window, x2, y2, x3, y3, color);
}
/**
 * @brief 画圆
 * @param window     展示窗口
 * @param x0         圆心x坐标
 * @param y0         圆心y坐标
 * @param r          半径
 * @param color      边框的颜色
*/
void Display_DrawCircle(display_window_t* window, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	int x = 0;
	int y = -r;
	int err = 2-2*r;
	int old_err;
	do{
		Display_Drawpoint(window, x0-x, y0+y, color); 
		Display_Drawpoint(window, x0-y, y0-x, color); 
		Display_Drawpoint(window, x0+x, y0-y, color); 
		Display_Drawpoint(window, x0+y, y0+x, color); 
		if ((old_err=err)<=x)	err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;	 
	} while(y<0);
}
/**
 * @brief 画实心圆
 * @param window     展示窗口
 * @param x0         圆心x坐标
 * @param y0         圆心y坐标
 * @param r          半径
 * @param color      填充的颜色
*/
void Display_DrawFillCircle(display_window_t* window, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	int x = 0;
	int y = -r;
	int err = 2-2*r;
	int old_err;
	int ChangeX = 1;
	do{
		if(ChangeX) {
			Display_DrawLine(window, x0-x, y0-y, x0-x, y0+y, color);
			Display_DrawLine(window, x0+x, y0-y, x0+x, y0+y, color);
		}
		ChangeX=(old_err=err)<=x;
		if (ChangeX)			err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;
	} while(y<=0);
}
/**
 * @brief 画圆角矩形
 * @param window     展示窗口
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param r          圆角半径
 * @param color      填充的颜色
*/
void Display_DrawRoundRect(display_window_t* window, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t color)
{
	int x = 0;
	int y = -r;
	int err = 2-2*r;
	int old_err;
	unsigned char temp;
	if(x1>x2) {
		temp=x1; x1=x2; x2=temp;
	}
	if(y1>y2) {
		temp=y1; y1=y2; y2=temp;
	}
	if (x2-x1 < r) return;
	if (y2-y1 < r) return;
	do{
		if(x){
			Display_Drawpoint(window, x1+r-x, y1+r+y, color); 
			Display_Drawpoint(window, x2-r+x, y1+r+y, color); 
			Display_Drawpoint(window, x1+r-x, y2-r-y, color); 
			Display_Drawpoint(window, x2-r+x, y2-r-y, color);
		}
		if ((old_err=err)<=x)	err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;	 
	} while(y<0);
	Display_DrawLine(window, x1+r,y1  ,x2-r,y1	,color);
	Display_DrawLine(window, x1+r,y2  ,x2-r,y2	,color);
	Display_DrawLine(window, x1  ,y1+r,x1  ,y2-r,color);
	Display_DrawLine(window, x2  ,y1+r,x2  ,y2-r,color);  
}
/**
 * @brief 画一个空心箭头
 * @param window     展示窗口
 * @param x0         开始x坐标
 * @param y0         开始y坐标
 * @param x1         结束x坐标
 * @param y1         结束y坐标
 * @param w          箭头宽度
 * @param color      填充的颜色
 * @note Thanks http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html
*/
void Display_DrawArrow(display_window_t* window, uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color)
{
	double Vx= x1 - x0;
	double Vy= y1 - y0;
	double v = sqrt(Vx*Vx+Vy*Vy);
	double Ux= Vx/v;
	double Uy= Vy/v;
	uint16_t L[2],R[2];
	L[0]= x1 - Uy*w - Ux*v;
	L[1]= y1 + Ux*w - Uy*v;
	R[0]= x1 + Uy*w - Ux*v;
	R[1]= y1 - Ux*w - Uy*v;
	Display_DrawLine(window, x1, y1, L[0], L[1], color);
	Display_DrawLine(window, x1, y1, R[0], R[1], color);
	Display_DrawLine(window, L[0], L[1], R[0], R[1], color);
}
/**
 * @brief 画一个实心箭头
 * @param window     展示窗口
 * @param x0         开始x坐标
 * @param y0         开始y坐标
 * @param x1         结束x坐标
 * @param y1         结束y坐标
 * @param w          箭头宽度
 * @param color      填充的颜色
*/
void Display_DrawFillArrow(display_window_t* window, uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color)
{
	double Vx= x1 - x0;
	double Vy= y1 - y0;
	double v = sqrt(Vx*Vx+Vy*Vy);
	double Ux= Vx/v;
	double Uy= Vy/v;
	uint16_t L[2],R[2];
	L[0]= x1 - Uy*w - Ux*v;
	L[1]= y1 + Ux*w - Uy*v;
	R[0]= x1 + Uy*w - Ux*v;
	R[1]= y1 - Ux*w - Uy*v;
	Display_DrawLine(window, x0, y0, x1, y1, color);
	Display_DrawLine(window, x1, y1, L[0], L[1], color);
	Display_DrawLine(window, x1, y1, R[0], R[1], color);
	Display_DrawLine(window, L[0], L[1], R[0], R[1], color);
	int ww;
	for(ww=w-1;ww>0;ww--) {
		L[0]= x1 - Uy*ww - Ux*v;
		L[1]= y1 + Ux*ww - Uy*v;
		R[0]= x1 + Uy*ww - Ux*v;
		R[1]= y1 - Ux*ww - Uy*v;
		Display_DrawLine(window, x1, y1, L[0], L[1], color);
		Display_DrawLine(window, x1, y1, R[0], R[1], color);
	}
}
/**
 * @brief 画一个字
 * @param window     展示窗口
 * @param font       字体文件指针
 * @param font       汉字GBK编码
 * @param x          x坐标
 * @param y          y坐标
 * @param color      文字颜色
 * @return 0绘画失败 >0紧挨的下个字体x坐标
*/
int Display_DrawChar(display_window_t* window, FontFile *fp, char* font, uint8_t x, uint8_t y, uint16_t color) 
{
	uint8_t code[32];
	bool res = GetFontCode(fp, font, code);
	if (!res) return 0;
    uint8_t *codeptr = code;
	uint8_t w,h;
	if (window->_font_fill) 
        Display_DrawFillRect(window, x, y, x+15, y+15, window->_font_fill_color);
	for(h=0;h<16;h++)
    {
		for(w=0;w<16;w++)
        {
            if (*codeptr & (0X80 >> (w % 8)))
                Display_Drawpoint(window, x+w, y+h, color);
            if ((h == 14 || h == 15) && window->_font_underline)
                Display_Drawpoint(window, x+w, y+h, window->_font_underline_color);
            if (w % 8 == 7) codeptr++;
		}
	}
	return (x+16);
}
/**
 * @brief 写一段话
 * @param window     展示窗口
 * @param font       字体文件指针
 * @param fonts      这段话的gbk编码
 * @param x          x坐标
 * @param y          y坐标
 * @param color      文字颜色
 * @return 0绘画失败 >0紧挨的下段话x坐标
*/
int Display_DrawString(display_window_t* window, FontFile *fp, char* fonts, uint16_t x, uint16_t y, uint16_t color)
{
	int length = strlen(fonts);
    char *font = malloc(sizeof(char) * 2);
    uint8_t isGBK,xx=x;
	for(int i=0;i<length;i+=2) 
    {
        isGBK = *font = fonts[i];
        if (isGBK > 0XFE || isGBK < 0X81)
		{
			i--;
			continue;
		}  
        *(font+1) = fonts[i+1]; 
        x = Display_DrawChar(window, fp, font, x, y, color);
        if(x+16>window->_w){x=xx;y+=16;}
	}
	return x;
}
/**
 * @brief 填充整个屏幕
 * @param tft        tft设备
 * @param color      填充颜色
*/
void Display_FillScreen(display_window_t* window, uint16_t color)
{
    Display_DrawFillRect(window, 0, 0, window->_w-1, window->_h-1, color);
}
/**
 * @brief 设置字体填充颜色
 * @param window     展示窗口
 * @param color      填充颜色
*/
void Display_SetFontFill(display_window_t* window, uint16_t color)
{
	window->_font_fill = true;
	window->_font_fill_color = color;
}
/**
 * @brief 关闭字体填充
 * @param window     展示窗口
*/
void Display_UnsetFontFill(display_window_t* window)
{
	window->_font_fill = false;
}
/**
 * @brief 设置字体下划线颜色
 * @param window     展示窗口
 * @param color      下划线颜色
*/
void Display_SetFontUnderLine(display_window_t* window, uint16_t color)
{
	window->_font_underline = true;
	window->_font_underline_color = color;
}
/**
 * @brief 关闭字体下划线
 * @param window     展示窗口
*/
void Display_UnsetFontUnderLine(display_window_t* window)
{
	window->_font_underline = false;
}