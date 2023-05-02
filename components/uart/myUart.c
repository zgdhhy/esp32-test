#include "myUart.h"

/**
 * @brief UART初始化
 * @param baud 串口的波特率
 * @note  注册UART驱动程序 配置UART 设置管脚
*/
void Uart_Init(int baud)
{
    uart_config_t config = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };
    uart_driver_install(UART_NUM,RX_BUF_SIZE,0,0,NULL,0);
    uart_param_config(UART_NUM, &config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS);
}
/**
 * @brief 从UART读出数据
 * @param buf    读出数据缓存区的指针
 * @param length 读出数据缓存区的大小
 * @return -1出错 >=0从UART FIFO读取的字节数
*/
int Uart_Read_Data(void *buf, uint32_t length)
{
    return uart_read_bytes(0, buf, length, 20 / portTICK_RATE_MS);
}