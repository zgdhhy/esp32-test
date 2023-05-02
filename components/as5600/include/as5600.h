#ifndef AS5600_H
#define AS5600_H
#include "driver/i2c.h"

#define IIC_SDA         18          //AS5600 sda 引脚
#define IIC_SCL         19          //AS5600 scl 引脚
#define IIC_PORT        I2C_NUM_0   //使用的I2C通道
#define IIC_HZ          400000      //AS5600 master 时钟频率
#define AS5600_ADDR     0x36        //AS5600 I2C 器件地址 

#define WRITE_BIT I2C_MASTER_WRITE  //I2C master write 
#define READ_BIT  I2C_MASTER_READ   //I2C master read 
#define ACK_EN true
#define ACK_DIS false 

void AS5600_Init();
void AS5600_Write_Data(uint8_t address, uint8_t data);
uint8_t AS5600_Read_Data(uint8_t address);
uint16_t AS5600_Get_Angel();
#endif