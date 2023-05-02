#ifndef INA226_H
#define INA226_H
#include "driver/i2c.h"
#include "esp_log.h"

#define WRITE_ADDR      0X80        //IIC写地址
#define READ_ADDR       0X81        //IIC读地址

#define Config_Reg      0x00        //配置寄存器 设置控制设备的运行模式 该寄存器控制并联和母线电压测量的转换时间设置以及使用的平均模式 控制什么信号被选择被测量的工作模式
#define ShuntV_Reg      0x01        //分流电压寄存器
#define BusV_Reg        0x02        //总线电压寄存器
#define Power_Reg       0x03        //功率寄存器
#define Current_Reg     0x04        //电流寄存器
#define Calib_Reg       0x05        //校准寄存器 为设备提供了产生测量差分电压的并联电阻的值它还设置当前寄存器的解析 编程这个寄存器设置Current_LSB和Power_LSB(Current_LSB 25倍) 该寄存器也适用于整个系统的校准
#define Mask_En_Reg     0x06        //掩码/启用寄存器 选择启用的功能来控制警报引脚以及该引脚如何工作
#define Alert_Reg       0x07        //警报限制寄存器 包含用于与掩码/启用寄存器中选择的寄存器进行比较的值，以确定是否超过了限制
#define Man_ID_Reg      0xFE        //制造商ID寄存器 存储制造商的唯一标识号 0x5449
#define ID_Reg          0xFF        //ID寄存器 存储模具的唯一标识号和修订ID 0x2260

#define IIC_SDA         18          //INA226 sda 引脚
#define IIC_SCL         19          //INA226 scl 引脚
#define IIC_HZ          400000      //INA226 master 时钟频率
#define IIC_PORT        I2C_NUM_0   //INA226 使用的I2C通道
#define ACK_EN          true        //打开ack
#define ACK_DIS         false       //关闭ack

typedef struct                      //ina226 iic驱动配置
{
    int iic_sda;                    //iic sda引脚
    int iic_scl;                    //iic scl引脚
    uint32_t iic_hz;                //iic 时钟频率
    i2c_port_t iic_port;            //iic 使用的I2C通道
} ina226_driver_t;

typedef struct                      //ina226 设备
{
	float Bus_voltage;			    //总线电压(mV)
	float Shunt_voltage;		    //分流电压(uV)
	float Current;		            //电流(mA)
	float power;                    //功率(mw)
    ina226_driver_t ina226_driver;  //ina226 iic驱动
} ina226_t;


#define INA226_INIT_CONFIG_DEFAULT() {          \
    .Bus_voltage = 0,                           \
    .Shunt_voltage = 0,                         \
    .Current = 0,                               \
    .power = 0,                                 \
    .ina226_driver.iic_sda = IIC_SDA,           \
    .ina226_driver.iic_scl = IIC_SCL,           \
    .ina226_driver.iic_hz = IIC_HZ,             \
    .ina226_driver.iic_port = IIC_PORT,         \
}

void INA226_Init(ina226_t ina226);
void INA226_Write_Data(ina226_driver_t ina226_driver,uint8_t address,uint16_t data);
uint16_t INA226_Read_Data(ina226_driver_t ina226_driver,uint8_t address);
uint16_t INA226_Get_RegID(ina226_t ina226);
uint16_t INA226_Get_RegCal(ina226_t ina226);
uint16_t INA226_Get_RegCurrent(ina226_t ina226);
uint16_t INA226_Get_RegShuntVoltage(ina226_t ina226);
uint16_t INA226_Get_RegBusVoltage(ina226_t ina226);
uint16_t INA226_Get_RegPower(ina226_t ina226);
void INA226_Get_BusVoltage(ina226_t *ina226);
void INA226_Get_ShuntVoltage(ina226_t *ina226);
void INA226_Get_Current(ina226_t *ina226);
void INA226_Get_Power(ina226_t *ina226);
void INA226_Get_Data(ina226_t *ina226);
#endif