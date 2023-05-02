#include "oled.h"

/**
 * @note
 * 0x80 表示这是一条指令
 * 后面紧跟相关命令 对于命令有两种方式 
 * 一种是命令和设置的数据存在于一条指令中  需要一条指令 
 * 另一种是命令先发送 后必须在跟设置的数据 需要两条指令
 * 
*/
static uint8_t OLED_Init_Cmd[] = {
    0x80,0xAE,//1--关闭oled显示

    0x80,0xD5,//2--设置显示时钟分频因子[3:0]/振荡器频率[7:4] 
    0x80,0xF0,// --设置的值80
    0x80,0x20,//2--设置内存寻址模式  0x00列地址/0x01行地址/0x02页地址(默认)
    0x80,0x02,// --设置的值
    0x80,0xA8,//2--设置多路传输比率(1 to 64)
    0x80,0x3F,// --设置的值
    0x80,0xDA,//2--设置列引脚硬件配置
    0x80,0x12,// --设置的值
    /* ----- 方向显示配置 ----- */
    0x80,0xA1,//1--列扫描顺序        0xa1 从左到右 0xa0 从右到左
    0x80,0xC8,//1--行扫描顺序        0xc8 从上到下 0xc8 从下到上
    /* ----- END ----- */
    0x80,0x40,//1--设置开始行地址 集映射RAM显示开始行[5:0]
    0x80,0x00,//1--设置低列地址      0x00 - 0x0f
    0x80,0x10,//1--设置高列地址      0x10 - 0x1f
    0x80,0xD3,//2--设置显示偏移	     0x00 - 0x3F
    0x80,0x00,// --设置的值
    0x80,0x81,//2--设置对比度        0x00 - oxff
    0x80,0xFF,// --设置的值cf
    0x80,0xD9,//2--设置预充电期间的持续时间
    0x80,0xF1,// --设置的值
    0x80,0xDB,//2--调整VCOMH调节器的输出
    0x80,0x40,// --设置的值
    0x80,0x8D,//2--电荷泵设置       0x10 禁用 0x18 启用
    0x80,0x14,// --设置的值
    0x80,0xA4,//1--全局显示开启      0xa4 恢复到RAM显示 0xa5全亮
    0x80,0xA6,//1--设置显示方式      0xa6 1亮0灭   0xa7 0亮1灭
    0x80,0xA6,//1--设置显示方式      0xa6 1亮0灭   0xa7 0亮1灭
    0x80,0X2E,//1--关闭滚动
    0x80,0xAF //1--打开oled显示 
};
/**
 * @brief 向OLED写入一条指令
 * @param Cmd 指令内容
*/
void OLED_Write_Cmd(uint8_t Cmd)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(OLED_ADDR<<1)|I2C_MASTER_WRITE,ACK_EN);
    i2c_master_write_byte(cmd,0x80,ACK_EN);
    i2c_master_write_byte(cmd,Cmd,ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(IIC_PORT,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}
/**
 * @brief 向OLED写入一条数据
 * @param Data 数据内容
*/
void OLED_Write_Data(uint8_t Data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(OLED_ADDR<<1)|I2C_MASTER_WRITE,ACK_EN);
    i2c_master_write_byte(cmd,0x40,ACK_EN);
    i2c_master_write_byte(cmd,Data,ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(IIC_PORT,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}
/**
 * @brief 向OLED写入一堆数据
 * @param data 数据缓存区的指针
 * @param size 缓存区的大小
 * @param isData 缓存中是否是数据
*/
void OLED_Write(uint8_t *data, size_t size, bool isData)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(OLED_ADDR<<1)|I2C_MASTER_WRITE,ACK_EN);
    if(isData) i2c_master_write_byte(cmd,0x40,ACK_EN);
    i2c_master_write(cmd, data, size, ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(IIC_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}
/**
 * @brief 设置OLED显存的指针
 * @param page_addr 设置页数[0-7]
 * @param col_addr  设置列数[0-127]
*/ 
void OLED_SetPos(uint8_t page_addr, uint16_t col_addr)
{
    if(page_addr>7)  page_addr = 7;
    if(col_addr>127) col_addr = 127;
    OLED_Write_Cmd(0xb0 | page_addr);
    OLED_Write_Cmd(0x00 | (col_addr & 0x00ff));
    OLED_Write_Cmd(0x10 | (col_addr >> 8));
}  
/**
 * @brief 更改显存寻址方式
 * @param page_addr   设置页数[0-7]
 * @param Write_Data  单字节内容
*/ 
void OLED_Set_Memory_Address_Mode(addressmode_t mode)
{
    OLED_Write_Cmd(0x20);
    OLED_Write_Cmd(mode);
}
/**
 * @brief 根据数据缓存区刷新一页
 * @param page_addr   设置页数[0-7]
 * @param refresh_page  数据缓存区的指针[<128]
*/ 
void OLED_Display_Page(uint8_t page_addr, uint8_t *refresh_page)
{
    OLED_SetPos(page_addr,0);
    OLED_Write(refresh_page,128,true);
}
/**
 * @brief 将一个字节的内容写入一页显存中
 * @param page_addr   设置页数[0-7]
 * @param Write_Data  单字节内容
*/ 
void OLED_Display_PageByte(uint8_t page_addr, uint8_t Write_Data)
{
    uint8_t refresh_page[128];
    for(size_t i = 0; i < 128; i++) 
        refresh_page[i] = Write_Data;
    OLED_Display_Page(page_addr,refresh_page);
}
/**
 * @brief 根据数据缓存区刷新整个屏幕
 * @param refresh_all  数据缓存区的指针[<128*8]
*/ 
void OLED_Display(uint8_t *refresh_all)
{
    OLED_SetPos(0,0);
    OLED_Set_Memory_Address_Mode(LEVEL_MODE);
    OLED_Write(refresh_all, 128*8,true);
    OLED_Set_Memory_Address_Mode(PAGE_MODE);
}
/**X
 * @brief 将一个字节的内容写入整个屏幕中
 * @param Write_Data  单字节内容
*/ 
void OLED_DisplayByte(uint8_t Write_Data)
{
    uint8_t refresh_all[128*8];
    for(size_t i = 0; i < 128*8; i++)
        refresh_all[i] = Write_Data;
    OLED_Display(refresh_all);
}
/**
 * @brief 清除屏幕
*/
void OLED_Clear(void)
{
    OLED_DisplayByte(0x00);
}
/**
 * @brief OLED初始化
 * @note  配置i2c驱动 执行初始化命令 清屏 
*/
void OLED_Init()
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = IIC_SDA,
        .scl_io_num = IIC_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = IIC_HZ
    };
    i2c_param_config(IIC_PORT,&i2c_config);
    i2c_driver_install(IIC_PORT,I2C_MODE_MASTER,0,0,0);
    OLED_Write(OLED_Init_Cmd,58,false);
    OLED_Clear();
}