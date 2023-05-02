#ifndef OLED_H
#define OLED_H

#include "driver/i2c.h"
#define IIC_SDA     18          //OLED sda 引脚
#define IIC_SCL     19          //OLED scl 引脚
#define IIC_PORT    I2C_NUM_0   //使用的I2C通道
#define IIC_HZ      400000      //OLED master 时钟频率
#define OLED_ADDR   0x3c        //OLED I2C 器件地址    

#define WRITE_BIT I2C_MASTER_WRITE  //I2C master write 
#define READ_BIT  I2C_MASTER_READ   //I2C master read 
#define ACK_EN true
#define ACK_DIS false 

typedef enum {
    LEVEL_MODE = 0x00,
    VERTICAL_MODE,
    PAGE_MODE
} addressmode_t;

void OLED_Write_Cmd(uint8_t Cmd);
void OLED_Write_Data(uint8_t Data);
void OLED_Write(uint8_t *data, size_t size, bool isData);

void OLED_SetPos(uint8_t page_addr, uint16_t col_addr);
void OLED_Set_Memory_Address_Mode(addressmode_t mode);

void OLED_Display_Page(uint8_t page_addr, uint8_t *refresh_page);
void OLED_Display_PageByte(uint8_t page_addr, uint8_t Write_Data);

void OLED_Display(uint8_t *refresh_all);
void OLED_DisplayByte(uint8_t Write_Data);

void OLED_Clear(void);
void OLED_Init();
#endif