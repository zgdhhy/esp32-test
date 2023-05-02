#include "ina226.h"

/**
 * @brief INA226向寄存器里写入数据
 * @param ina226 ina226设备
*/
void INA226_Init(ina226_t ina226)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = ina226.ina226_driver.iic_sda,
        .scl_io_num = ina226.ina226_driver.iic_scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = ina226.ina226_driver.iic_hz
    };
    i2c_param_config(ina226.ina226_driver.iic_port,&i2c_config);
    i2c_driver_install(ina226.ina226_driver.iic_port,I2C_MODE_MASTER,0,0,0);
    INA226_Write_Data(ina226.ina226_driver,Config_Reg,0x4127);//设置转换时间204us,求平均值次数128，采样时间为204*128，设置模式为分流和总线连续模式
	INA226_Write_Data(ina226.ina226_driver,Calib_Reg,0x0A00);//设置分流电压转电流转换参数
}
/**
 * @brief INA226向寄存器里写入数据
 * @param ina226_driver  ina226 iic驱动
 * @param address 寄存器地址
 * @param data    写入数据
*/
void INA226_Write_Data(ina226_driver_t ina226_driver,uint8_t address,uint16_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,WRITE_ADDR,ACK_EN);
    i2c_master_write_byte(cmd,address,ACK_EN);
    i2c_master_write_byte(cmd,(uint8_t)((data&0xFF00)>>8),ACK_EN);
    i2c_master_write_byte(cmd,(uint8_t)(data&0x00FF),ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(ina226_driver.iic_port,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}
/**
 * @brief INA226从寄存器里读出数据
 * @param ina226_driver  ina226 iic驱动
 * @param address 寄存器地址
 * @return 读出的数据
*/
uint16_t INA226_Read_Data(ina226_driver_t ina226_driver,uint8_t address)
{
    uint8_t buf[2];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,WRITE_ADDR,ACK_EN);
    i2c_master_write_byte(cmd,address,ACK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,READ_ADDR,ACK_EN);
    i2c_master_read(cmd,buf,2,ACK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(ina226_driver.iic_port,cmd,1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return (buf[0]<<8&0xff00) | (buf[1]&0x00ff);
}
/**
 * @brief 获取ina226的id
 * @param ina226  ina226设备
 * @return 获取的id
*/
uint16_t INA226_Get_RegID(ina226_t ina226)
{	
	return INA226_Read_Data(ina226.ina226_driver,ID_Reg);
}
/**
 * @brief 获取ina226的校准值
 * @param ina226  ina226设备
 * @return 获取的校准值
*/
uint16_t INA226_Get_RegCal(ina226_t ina226)
{
	return INA226_Read_Data(ina226.ina226_driver,Calib_Reg);
}
/**
 * @brief 获取ina226电流寄存器的值 1ma/bit
 * @param ina226  ina226设备
 * @return 读出的数据
*/
uint16_t INA226_Get_RegCurrent(ina226_t ina226)
{
    uint16_t temp=0;	
	temp = INA226_Read_Data(ina226.ina226_driver,Current_Reg);
	if(temp&0x8000)	temp = ~(temp - 1);	
	return temp;
}
/**
 * @brief 获取ina226分流电压寄存器的值 2.5uV/bit
 * @param ina226  ina226设备
 * @return 读出的数据
*/
uint16_t INA226_Get_RegShuntVoltage(ina226_t ina226)
{
	int16_t temp=0;
	temp = INA226_Read_Data(ina226.ina226_driver,ShuntV_Reg);
	if(temp&0x8000)	temp = ~(temp - 1);	
	return temp;	
}
/**
 * @brief 获取ina226总线电压寄存器的值 1.25mV/bit
 * @param ina226  ina226设备
 * @return 读出的数据
*/
uint16_t INA226_Get_RegBusVoltage(ina226_t ina226)
{
	uint16_t temp=0;	
	temp = INA226_Read_Data(ina226.ina226_driver,BusV_Reg);
	return temp;	
}
/**
 * @brief 获取ina226功率寄存器的值 25mw/bit
 * @param ina226  ina226设备
 * @return 读出的数据
*/
uint16_t INA226_Get_RegPower(ina226_t ina226)
{
	uint16_t temp=0;	
	temp = INA226_Read_Data(ina226.ina226_driver,Power_Reg);
	return temp;	
}

/**
 * @brief 获取ina226总线电压
 * @param ina226[0ut] ina226设备
*/
void INA226_Get_BusVoltage(ina226_t *ina226)//mV
{
	ina226->Bus_voltage = INA226_Get_RegBusVoltage(*ina226)*1.25f;
}
/**
 * @brief 获取ina226分流电压
 * @param ina226[0ut] ina226设备
*/
void INA226_Get_ShuntVoltage(ina226_t *ina226)//uV
{
	ina226->Shunt_voltage = INA226_Get_RegShuntVoltage(*ina226)*2.5f;
}
/**
 * @brief 获取ina226电流
 * @param ina226[0ut] ina226设备
*/
void INA226_Get_Current(ina226_t *ina226)//mA
{
	ina226->Current = INA226_Get_RegCurrent(*ina226)*1.0f;
}
/**
 * @brief 获取ina226功率
 * @param ina226[0ut] ina226设备
*/
void INA226_Get_Power(ina226_t *ina226)//mw
{
	ina226->power = INA226_Get_RegPower(*ina226)*25.0f;
}
/**
 * @brief 获取ina226功率寄存器的值 25mw/bit
 * @param ina226[0ut] ina226设备
*/
void INA226_Get_Data(ina226_t *ina226)
{
	INA226_Get_BusVoltage(ina226);			//mV
	INA226_Get_ShuntVoltage(ina226);		//uV
	INA226_Get_Current(ina226);		        //mA
    INA226_Get_Power(ina226);	            //mw
}
