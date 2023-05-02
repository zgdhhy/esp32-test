#ifndef MYUART_H
#define MYUART_H

#include "driver/uart.h"
// 在连接的时候 rx->tx  tx->rx 
#define UART_NUM  UART_NUM_0              //使用的串口
#define UART_TXD  UART_PIN_NO_CHANGE      //串口tx引脚    UART_PIN_NO_CHANGE默认脚
#define UART_RXD  UART_PIN_NO_CHANGE      //串口rx引脚
#define UART_RTS  UART_PIN_NO_CHANGE      //串口rts引脚
#define UART_CTS  UART_PIN_NO_CHANGE      //串口cts引脚

#define RX_BUF_SIZE 60*60*8 + 200         //rxbuf缓冲区大小

void Uart_Init(int baud);
int Uart_Read_Data(void *buf, uint32_t length);
#endif