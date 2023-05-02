#include "as5600.h"

/**
 * @brief AS5600初始化
 * @note  配置i2c驱动
*/
void AS5600_Init()
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
}
/**
 * @brief AS5600向寄存器里写入数据
 * @param address 寄存器地址
 * @param data    写入数据
*/
void AS5600_Write_Data(uint8_t address, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(AS5600_ADDR << 1)|I2C_MASTER_WRITE,ACK_EN);
    i2c_master_write_byte(cmd,address,ACK_EN);
    i2c_master_write_byte(cmd,data,ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(IIC_PORT,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}
/**
 * @brief AS5600从寄存器里读出数据
 * @param address 寄存器地址
 * @return 读出的数据
*/
uint8_t AS5600_Read_Data(uint8_t address)
{
    uint8_t data = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(AS5600_ADDR << 1)|I2C_MASTER_WRITE,ACK_EN);
    i2c_master_write_byte(cmd,address,ACK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(AS5600_ADDR << 1)|I2C_MASTER_READ,ACK_EN);
    i2c_master_read_byte(cmd,&data,ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(IIC_PORT,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return data;
}
/**
 * @brief AS5600从寄存器里读出角度
 * @return 读出的角度
*/
uint16_t AS5600_Get_Angel()
{
    uint16_t angel = 0;
    angel = AS5600_Read_Data(0x0E);
    angel <<= 8;
    angel |= AS5600_Read_Data(0x0F);
    return angel;
}