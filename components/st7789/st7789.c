#include "st7789.h"

static const char *TAG = "ST7789";
/**
 * @brief 对spi设备引脚进行初始化
 * @param tft       tft设备
 * @param MOSI_GPIO SDA引脚
 * @param SCLK_GPIO SCL引脚
 * @param DC_GPIO   DC引脚
 * @param CS_GPIO   CS引脚 -1不使用
 * @param RES_GPIO  RES引脚-1不使用
 * @param BLK_GPIO  BLK引脚-1不使用
*/
void spi_st7789_init(TFT_t *tft,int8_t MOSI_GPIO,int8_t SCLK_GPIO,int8_t DC_GPIO,int8_t CS_GPIO,int8_t RES_GPIO, int8_t BLK_GPIO)
{
    if(CS_GPIO >= 0)
    {
        gpio_reset_pin(CS_GPIO);
        gpio_set_direction(CS_GPIO,GPIO_MODE_OUTPUT);
        gpio_set_level(CS_GPIO,0);
    }
    gpio_reset_pin(DC_GPIO);
    gpio_set_direction(DC_GPIO,GPIO_MODE_OUTPUT);
    gpio_set_level(DC_GPIO,0);
    if(RES_GPIO >= 0)
    {
        gpio_reset_pin(RES_GPIO);
        gpio_set_direction(RES_GPIO,GPIO_MODE_OUTPUT);
        gpio_set_level(RES_GPIO,1);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_set_level(RES_GPIO,0);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_set_level(RES_GPIO,1);
    }
    if(BLK_GPIO >= 0)
    {
        gpio_reset_pin(BLK_GPIO);
        gpio_set_direction(BLK_GPIO,GPIO_MODE_OUTPUT);
        gpio_set_level(BLK_GPIO,0);
    }
    spi_bus_config_t buscfg = {
        .mosi_io_num = MOSI_GPIO,
        .miso_io_num = -1,
        .sclk_io_num = SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0
    };
    spi_bus_initialize(HOST_ID,&buscfg,SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_40M;
    devcfg.queue_size = 7;
    devcfg.mode = 2;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;
    if(CS_GPIO >= 0) devcfg.spics_io_num = CS_GPIO;
    else devcfg.spics_io_num = -1;

    spi_device_handle_t handle;
    spi_bus_add_device(HOST_ID,&devcfg,&handle);
    tft->_dc = DC_GPIO;
    tft->_blk = BLK_GPIO;
    tft->_SPIHandle = handle;
}
/**
 * @brief spi设备发送一段数据
 * @param handle    设备句柄
 * @param data      数据缓冲器指针
 * @param length    缓冲区长度
*/
void spi_st7789_write_byte(spi_device_handle_t handle,const uint8_t *data,size_t length)
{
    spi_transaction_t transaction;
    if(length > 0)
    {
        memset(&transaction,0,sizeof(spi_transaction_t));
        transaction.length = length * 8;
        transaction.tx_buffer = data;
        spi_device_transmit(handle,&transaction);
    }
}
/**
 * @brief spi设备发送命令
 * @param tft      tft设备
 * @param cmd      发送的命令
*/
void spi_st7789_write_command(TFT_t *tft,uint8_t cmd)
{
    gpio_set_level(tft->_dc,SPI_CMD_MODE);
    spi_st7789_write_byte(tft->_SPIHandle,&cmd,1);
}
/**
 * @brief spi设备发送8位数据
 * @param tft      tft设备
 * @param data     发送的数据
*/
void spi_st7789_write_data(TFT_t *tft,uint8_t data)
{
    gpio_set_level(tft->_dc,SPI_DATA_MODE);
    spi_st7789_write_byte(tft->_SPIHandle,&data,1);
}
/**
 * @brief spi设备发送一个地址
 * @param tft      tft设备
 * @param addr1    发送的地址1
 * @param addr2    发送的地址2
*/
void spi_st7789_write_addr(TFT_t *tft, uint16_t addr1, uint16_t addr2)
{
	static uint8_t Byte[4];
	Byte[0] = (addr1 >> 8) & 0xFF;
	Byte[1] = addr1 & 0xFF;
	Byte[2] = (addr2 >> 8) & 0xFF;
	Byte[3] = addr2 & 0xFF;
	gpio_set_level(tft->_dc, SPI_DATA_MODE);
	spi_st7789_write_byte(tft->_SPIHandle, Byte, 4);
}
/**
 * @brief spi设备写一种颜色
 * @param tft      tft设备
 * @param color    发送的颜色
 * @param size     该种颜色写多少个
*/
void spi_st7789_write_color(TFT_t *tft, uint16_t color, uint16_t size)
{
	static uint8_t Byte[1024];
	int index = 0;
	for(int i=0;i<size;i++) {
		Byte[index++] = (color >> 8) & 0xFF;
		Byte[index++] = color & 0xFF;
	}
	gpio_set_level(tft->_dc, SPI_DATA_MODE);
	spi_st7789_write_byte(tft->_SPIHandle, Byte, size*2);
}
/**
 * @brief spi设备写一堆颜色
 * @param tft      tft设备
 * @param colors   发送的颜色
 * @param size     发送多少颜色
*/
void spi_st7789_write_colors(TFT_t *tft, uint16_t *colors, uint16_t size)
{
	static uint8_t Byte[1024];
	int index = 0;
	for(int i=0;i<size;i++) {
		Byte[index++] = (colors[i] >> 8) & 0xFF;
		Byte[index++] = colors[i] & 0xFF;
	}
	gpio_set_level(tft->_dc, SPI_DATA_MODE);
	spi_st7789_write_byte(tft->_SPIHandle, Byte, size*2);
}
/**
 * @brief 设备初始化
 * @param tft        tft设备
 * @param width      设备的宽
 * @param height     设备的高
 * @param offsetx    设备的x偏移量
 * @param offsety    设备的y偏移量
*/
void ST7789_Init(TFT_t * tft, int width, int height, int offsetx, int offsety)
{
    ESP_LOGI(TAG,"ST7789_Init");
    tft->_width = width;
	tft->_height = height;
	tft->_offsetx = offsetx;
	tft->_offsety = offsety;
	tft->_font_fill = false;
	tft->_font_underline = false;

	spi_st7789_write_command(tft, 0x01);	//Software Reset
	vTaskDelay(150 / portTICK_PERIOD_MS);

	spi_st7789_write_command(tft, 0x11);	//Sleep Out
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	spi_st7789_write_command(tft, 0x3A);	//Interface Pixel Format
	spi_st7789_write_data(tft, 0x55);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	
	spi_st7789_write_command(tft, 0x36);	//Memory Data Access Control
	spi_st7789_write_data(tft, 0x00);

	spi_st7789_write_command(tft, 0x2A);	//Column Address Set
	spi_st7789_write_data(tft, 0x00);
	spi_st7789_write_data(tft, 0x00);
	spi_st7789_write_data(tft, 0x00);
	spi_st7789_write_data(tft, 0xF0);

	spi_st7789_write_command(tft, 0x2B);	//Row Address Set
	spi_st7789_write_data(tft, 0x00);
	spi_st7789_write_data(tft, 0x00);
	spi_st7789_write_data(tft, 0x00);
	spi_st7789_write_data(tft, 0xF0);

	spi_st7789_write_command(tft, 0x21);	//Display Inversion On
	vTaskDelay(10 / portTICK_PERIOD_MS);

	spi_st7789_write_command(tft, 0x13);	//Normal Display Mode On
	vTaskDelay(10 / portTICK_PERIOD_MS);

	spi_st7789_write_command(tft, 0x29);	//Display ON
	vTaskDelay(200 / portTICK_PERIOD_MS);

	if(tft->_blk >= 0) gpio_set_level( tft->_blk, 1 );
    ST7789_FillScreen(tft, BLACK);
}
/**
 * @brief 画一个点
 * @param tft        tft设备
 * @param x          点的x坐标
 * @param y          点的y坐标
 * @param color      点的颜色
*/
void ST7789_Drawpoint(TFT_t * tft, uint16_t x, uint16_t y, uint16_t color)
{
	if (x >= tft->_width) return;
	if (y >= tft->_height) return;

	uint16_t _x = x + tft->_offsetx;
	uint16_t _y = y + tft->_offsety;

	spi_st7789_write_command(tft, 0x2A);	// set column(x) address
	spi_st7789_write_addr(tft, _x, _x);
	spi_st7789_write_command(tft, 0x2B);	// set Page(y) address
	spi_st7789_write_addr(tft, _y, _y);
	spi_st7789_write_command(tft, 0x2C);	//	Memory Write
	spi_st7789_write_color(tft, color,1);
}
/**
 * @brief 画一堆点
 * @param tft        tft设备
 * @param x          开始x坐标
 * @param y          开始y坐标
 * @param size       多少个点啊[colors数组的尺寸]
 * @param colors     颜色堆数组
*/
void ST7789_DrawMultipoint(TFT_t * tft, uint16_t x,uint16_t y,uint16_t size,uint16_t *colors)
{
	if (x+size > tft->_width) return;
	if (y >= tft->_height) return;

	uint16_t _x1 = x + tft->_offsetx;
	uint16_t _x2 = _x1 + size;
	uint16_t _y = y + tft->_offsety;

	spi_st7789_write_command(tft, 0x2A);	// set column(x) address
	spi_st7789_write_addr(tft, _x1, _x2);
	spi_st7789_write_command(tft, 0x2B);	// set Page(y) address
	spi_st7789_write_addr(tft, _y, _y);
	spi_st7789_write_command(tft, 0x2C);	//	Memory Write
	spi_st7789_write_colors(tft, colors, size);
}
/**
 * @brief 画一条线
 * @param tft        tft设备
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param color      线的颜色
*/
void ST7789_DrawLine(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) 
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
	if ( dx > dy ) {
		E = -dx;
		for ( i = 0 ; i <= dx ; i++ ) {
			ST7789_Drawpoint(tft, x1, y1, color);
			x1 += sx;
			E += 2 * dy;
			if ( E >= 0 ) {
			y1 += sy;
			E -= 2 * dx;
		}
	}
	/* inclination >= 1 */
	} else {
		E = -dy;
		for ( i = 0 ; i <= dy ; i++ ) {
			ST7789_Drawpoint(tft, x1, y1, color);
			y1 += sy;
			E += 2 * dx;
			if ( E >= 0 ) {
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}
/**
 * @brief 画一个矩形
 * @param tft        tft设备
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param color      边框颜色
*/
void ST7789_DrawRect(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) 
{
	ST7789_DrawLine(tft, x1, y1, x2, y1, color);
	ST7789_DrawLine(tft, x2, y1, x2, y2, color);
	ST7789_DrawLine(tft, x2, y2, x1, y2, color);
	ST7789_DrawLine(tft, x1, y2, x1, y1, color);
}
/**
 * @brief 画一个实心矩形
 * @param tft        tft设备
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param color      填充的颜色
 * @note 对角矩阵
*/
void ST7789_DrawFillRect(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) 
{
	if (x1 >= tft->_width) return;
	if (x2 >= tft->_width) x2=tft->_width-1;
	if (y1 >= tft->_height) return;
	if (y2 >= tft->_height) y2=tft->_height-1;

	uint16_t _x1 = x1 + tft->_offsetx;
	uint16_t _x2 = x2 + tft->_offsetx;
	uint16_t _y1 = y1 + tft->_offsety;
	uint16_t _y2 = y2 + tft->_offsety;

	spi_st7789_write_command(tft, 0x2A);
	spi_st7789_write_addr(tft, _x1, _x2);
	spi_st7789_write_command(tft, 0x2B);
	spi_st7789_write_addr(tft, _y1, _y2);
	spi_st7789_write_command(tft, 0x2C);
    uint16_t size = _y2-_y1+1;
	for(int i=_x1;i<=_x2;i++) spi_st7789_write_color(tft, color, size);
}
/**
 * @brief 画一个有角度的矩形
 * @param tft        tft设备
 * @param xc         矩形中心x坐标
 * @param yc         矩形中心y坐标
 * @param w          矩形宽
 * @param h          矩形高
 * @param angle      旋转的角度
 * @param color      填充的颜色
*/
void ST7789_DrawRectAngle(TFT_t *tft, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color)
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

	ST7789_DrawLine(tft, x1, y1, x2, y2, color);
	ST7789_DrawLine(tft, x1, y1, x3, y3, color);
	ST7789_DrawLine(tft, x2, y2, x4, y4, color);
	ST7789_DrawLine(tft, x3, y3, x4, y4, color);
}
/**
 * @brief 画一个有角度的三角形
 * @param tft        tft设备
 * @param xc         三角形中心x坐标
 * @param yc         三角形中心y坐标
 * @param w          三角形底
 * @param h          三角形高
 * @param angle      旋转的角度
 * @param color      边框的颜色
*/
void ST7789_DrawTriangle(TFT_t *tft, uint16_t xc, uint16_t yc, uint16_t w, uint16_t h, uint16_t angle, uint16_t color)
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

	ST7789_DrawLine(tft, x1, y1, x2, y2, color);
	ST7789_DrawLine(tft, x1, y1, x3, y3, color);
	ST7789_DrawLine(tft, x2, y2, x3, y3, color);
}
/**
 * @brief 画圆
 * @param tft        tft设备
 * @param x0         圆心x坐标
 * @param y0         圆心y坐标
 * @param r          半径
 * @param color      边框的颜色
*/
void ST7789_DrawCircle(TFT_t *tft, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	int x = 0;
	int y = -r;
	int err = 2-2*r;
	int old_err;
	do{
		ST7789_Drawpoint(tft, x0-x, y0+y, color); 
		ST7789_Drawpoint(tft, x0-y, y0-x, color); 
		ST7789_Drawpoint(tft, x0+x, y0-y, color); 
		ST7789_Drawpoint(tft, x0+y, y0+x, color); 
		if ((old_err=err)<=x)	err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;	 
	} while(y<0);
}
/**
 * @brief 画实心圆
 * @param tft        tft设备
 * @param x0         圆心x坐标
 * @param y0         圆心y坐标
 * @param r          半径
 * @param color      填充的颜色
*/
void ST7789_DrawFillCircle(TFT_t *tft, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
	int x = 0;
	int y = -r;
	int err = 2-2*r;
	int old_err;
	int ChangeX = 1;
	do{
		if(ChangeX) {
			ST7789_DrawLine(tft, x0-x, y0-y, x0-x, y0+y, color);
			ST7789_DrawLine(tft, x0+x, y0-y, x0+x, y0+y, color);
		}
		ChangeX=(old_err=err)<=x;
		if (ChangeX)			err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;
	} while(y<=0);
}
/**
 * @brief 画圆角矩形
 * @param tft        tft设备
 * @param x1         开始x坐标
 * @param y1         开始y坐标
 * @param x2         结束x坐标
 * @param y2         结束y坐标
 * @param r          圆角半径
 * @param color      填充的颜色
*/
void ST7789_DrawRoundRect(TFT_t *tft, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t color)
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
			ST7789_Drawpoint(tft, x1+r-x, y1+r+y, color); 
			ST7789_Drawpoint(tft, x2-r+x, y1+r+y, color); 
			ST7789_Drawpoint(tft, x1+r-x, y2-r-y, color); 
			ST7789_Drawpoint(tft, x2-r+x, y2-r-y, color);
		}
		if ((old_err=err)<=x)	err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;	 
	} while(y<0);
	ST7789_DrawLine(tft, x1+r,y1  ,x2-r,y1	,color);
	ST7789_DrawLine(tft, x1+r,y2  ,x2-r,y2	,color);
	ST7789_DrawLine(tft, x1  ,y1+r,x1  ,y2-r,color);
	ST7789_DrawLine(tft, x2  ,y1+r,x2  ,y2-r,color);  
}
/**
 * @brief 画一个空心箭头
 * @param tft        tft设备
 * @param x0         开始x坐标
 * @param y0         开始y坐标
 * @param x1         结束x坐标
 * @param y1         结束y坐标
 * @param w          箭头宽度
 * @param color      填充的颜色
 * @note Thanks http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html
*/
void ST7789_DrawArrow(TFT_t *tft, uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color)
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
	ST7789_DrawLine(tft, x1, y1, L[0], L[1], color);
	ST7789_DrawLine(tft, x1, y1, R[0], R[1], color);
	ST7789_DrawLine(tft, L[0], L[1], R[0], R[1], color);
}
/**
 * @brief 画一个实心箭头
 * @param tft        tft设备
 * @param x0         开始x坐标
 * @param y0         开始y坐标
 * @param x1         结束x坐标
 * @param y1         结束y坐标
 * @param w          箭头宽度
 * @param color      填充的颜色
*/
void ST7789_DrawFillArrow(TFT_t *tft, uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color)
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
	ST7789_DrawLine(tft, x0, y0, x1, y1, color);
	ST7789_DrawLine(tft, x1, y1, L[0], L[1], color);
	ST7789_DrawLine(tft, x1, y1, R[0], R[1], color);
	ST7789_DrawLine(tft, L[0], L[1], R[0], R[1], color);
	int ww;
	for(ww=w-1;ww>0;ww--) {
		L[0]= x1 - Uy*ww - Ux*v;
		L[1]= y1 + Ux*ww - Uy*v;
		R[0]= x1 + Uy*ww - Ux*v;
		R[1]= y1 - Ux*ww - Uy*v;
		ST7789_DrawLine(tft, x1, y1, L[0], L[1], color);
		ST7789_DrawLine(tft, x1, y1, R[0], R[1], color);
	}
}
/**
 * @brief 画一个字
 * @param tft        tft设备
 * @param fp         字体文件指针
 * @param font       汉字GBK编码
 * @param x          x坐标
 * @param y          y坐标
 * @param color      文字颜色
 * @return 0绘画失败 >0紧挨的下个字体x坐标
*/
int ST7789_DrawChar(TFT_t *tft, FontFile *fp, char* font, uint8_t x, uint8_t y, uint16_t color) 
{
    FontCode* fcode = malloc(sizeof(FontCode));
	bool res = GetFontCode(fp, font, fcode);
	if (!res)
    {
        free(fcode);
        ESP_LOGI(TAG, "ST7789_DrawChar File can not find the word that need to print");
        return 0;
    }
    uint8_t *codeptr = fcode->code;
    uint8_t codew = fcode->wight;
	if (tft->_font_fill) 
        ST7789_DrawFillRect(tft, x, y, x+fp->wight-1, y+fp->height-1, tft->_font_fill_color);
	for(uint8_t h=0;h<fcode->height;h++)
    {
		for(uint8_t w=0;w<fcode->wight;w++)
        {
            if (*codeptr & (0X80 >> (w % 8)))
                ST7789_Drawpoint(tft, x+w, y+h, color);
            if ((h == fp->height-1 || h == fp->height-2) && tft->_font_underline)
                ST7789_Drawpoint(tft, x+w, y+h, tft->_font_underline_color);
            if (w % 8 == 7) codeptr++;
		}
	}
    free(fcode);
	return (x+codew);
}
/**
 * @brief 写一段话
 * @param tft        tft设备
 * @param font       字体文件指针
 * @param fonts      这段话的gbk编码
 * @param x          x坐标
 * @param y          y坐标
 * @param color      文字颜色
 * @return 0绘画失败 >0紧挨的下段话x坐标
*/
int ST7789_DrawString(TFT_t *tft, FontFile *fp, char* fonts, uint16_t x, uint16_t y, uint16_t color)
{
	int length = strlen(fonts);
    char *font = malloc(sizeof(char) * 2);
    uint8_t isGBK,xx=x;
	for(int i=0;i<length;i+=2) 
    {
        isGBK = *font = fonts[i];
        if (isGBK > 0XFE || isGBK < 0X81) i--;
        *(font+1) = fonts[i+1]; 
        x = ST7789_DrawChar(tft, fp, font, x, y, color);
        if(x+fp->wight>tft->_width){x=xx;y+=fp->height;}
	}
	return x;
}

/**
 * @brief 关闭显示
 * @param tft        tft设备
*/
void ST7789_DisplayOff(TFT_t *tft)
{
	spi_st7789_write_command(tft, 0x28);
}
/**
 * @brief 打开显示
 * @param tft        tft设备
*/
void ST7789_DisplayOn(TFT_t *tft)
{
	spi_st7789_write_command(tft, 0x29);
}
/**
 * @brief 填充整个屏幕
 * @param tft        tft设备
 * @param color      填充颜色
*/
void ST7789_FillScreen(TFT_t *tft, uint16_t color) 
{
	ST7789_DrawFillRect(tft, 0, 0, tft->_width-1, tft->_height-1, color);
}
/**
 * @brief 设置字体填充颜色
 * @param tft        tft设备
 * @param color      填充颜色
*/
void ST7789_SetFontFill(TFT_t *tft, uint16_t color)
{
	tft->_font_fill = true;
	tft->_font_fill_color = color;
}
/**
 * @brief 关闭字体填充
 * @param tft        tft设备
*/
void ST7789_UnsetFontFill(TFT_t *tft)
{
	tft->_font_fill = false;
}
/**
 * @brief 设置字体下划线颜色
 * @param tft        tft设备
 * @param color      下划线颜色
*/
void ST7789_SetFontUnderLine(TFT_t *tft, uint16_t color)
{
	tft->_font_underline = true;
	tft->_font_underline_color = color;
}
/**
 * @brief 关闭字体下划线
 * @param tft        tft设备
*/
void ST7789_UnsetFontUnderLine(TFT_t *tft)
{
	tft->_font_underline = false;
}
/**
 * @brief 打开背光
 * @param tft        tft设备
*/
void ST7789_BacklightOn(TFT_t *tft)
{
	if(tft->_blk >= 0) {
		gpio_set_level( tft->_blk, 1 );
	}
}
/**
 * @brief 关闭背光
 * @param tft        tft设备
*/
void ST7789_BacklightOff(TFT_t *tft)
{
	if(tft->_blk >= 0) {
		gpio_set_level( tft->_blk, 0 );
	}
}
/**
 * @brief 打开反转展示
 * @param tft        tft设备
*/
void ST7789_InversionOn(TFT_t *tft)
{
	spi_st7789_write_command(tft, 0x21);
}
/**
 * @brief 关闭反转展示
 * @param tft        tft设备
*/
void ST7789_InversionOff(TFT_t *tft)
{
	spi_st7789_write_command(tft, 0x20);
}
/**
 * @brief RGB转化为RGB565(16位)的形式
 * @param r     红色
 * @param g     绿色
 * @param b     蓝色
 * @return RGB565
 * @note  RGB565 is R(5)+G(6)+B(5)=16bit color format
 *        Bit image "RRRRRGGGGGGBBBBB"
*/
uint16_t rgb565_conv(uint16_t r,uint16_t g,uint16_t b) 
{
	return (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}