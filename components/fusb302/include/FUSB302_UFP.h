/**
 * FUSB302_UFP.h
 *
 *  Updated on: Jan 4, 2021
 *      Author: Ryan Ma
 *
 * Minimalist USB PD implement with only UFP(device) functionality
 * Requires only stdint.h and string.h
 * No use of bit-field for better cross-platform compatibility
 *
 * FUSB302 can support PD3.0 with limitations and workarounds
 * - Do not have enough FIFO for unchunked message, use chunked message instead
 * - VBUS sense low threshold at 4V, disable vbus_sense if request PPS below 4V
 * 
 */

#ifndef FUSB302_UFP_H
#define FUSB302_UFP_H

#include <stdint.h>

enum {
    FUSB302_SUCCESS             = 0,
    FUSB302_BUSY                = (1 << 0),
    FUSB302_ERR_PARAM           = (1 << 1),
    FUSB302_ERR_DEVICE_ID       = (1 << 2),
    FUSB302_ERR_READ_DEVICE     = (1 << 3),
    FUSB302_ERR_WRITE_DEVICE    = (1 << 4)
};
typedef uint8_t FUSB302_ret_t;

#define FUSB302_EVENT_ATTACHED          (1 << 0)    //Type-c连接事件
#define FUSB302_EVENT_DETACHED          (1 << 1)    //Type-c断开事件
#define FUSB302_EVENT_RX_SOP            (1 << 2)    //已接受缓冲区内容
#define FUSB302_EVENT_GOOD_CRC_SENT     (1 << 3)    //已发送一个GOODCRC报文
typedef uint8_t FUSB302_event_t;

typedef struct {
    /* 由用户设置 */
    uint8_t i2c_address;    //设备i2c地址
    // i2c读函数
    FUSB302_ret_t (*i2c_read)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
    // i2c写函数
    FUSB302_ret_t (*i2c_write)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
    // 延时函数 ms级别
    FUSB302_ret_t (*delay_ms)(uint32_t t);

    /* 这个库使用的数据 */
    const char * err_msg;   // 错误信息
    uint16_t rx_header;     // 消息头
    uint8_t rx_buffer[32];  // 接受缓冲区 最高接受8个32位数据
    uint8_t reg_control[15];// FUSB302控制寄存器缓冲区
    uint8_t reg_status[7];  // FUSB302状态寄存器缓冲区
    
    uint8_t interrupta;     // FUSB302 a中断寄存器 缓冲区
    uint8_t interruptb;     // FUSB302 b中断寄存器 缓冲区
    uint8_t cc1;            // cc1 线上的电压
    uint8_t cc2;            // cc2 线上的电压
    uint8_t state;          // FUSB302的状态 连接或断开
    uint8_t vbus_sense;     // 电源感知 使能来监听电源中断信号(VBUSOK为0 表示连接断开)
} FUSB302_dev_t;

// 返回最近错误信息
static inline const char * FUSB302_get_last_err_msg(FUSB302_dev_t *dev) { return dev->err_msg; }
/* 初始化 FUSB302设备 1观察设备I2C设置是否正常 2初始化消息头 接受缓冲区和FUSB302的状态默认断开
3将寄存器全部恢复默认设置 并重新获取寄存器值 4 下拉cc1 cc2 配置测量通道 5 配置若没有收到GOODCRC 3次重传
6 打开中断 7打开电源 8设置vbus_sense为1 err_msg为空*/ 
FUSB302_ret_t FUSB302_init            (FUSB302_dev_t *dev);
// 重置 FUSB302 tx rx 逻辑？
FUSB302_ret_t FUSB302_pd_reset        (FUSB302_dev_t *dev);
// enable为1 拉低cc1 cc2
FUSB302_ret_t FUSB302_pdwn_cc         (FUSB302_dev_t *dev, uint8_t enable);
// 设置vbus_sense的值 立即起作用
FUSB302_ret_t FUSB302_set_vbus_sense  (FUSB302_dev_t *dev, uint8_t enable);
// 获取FUSB302 ID信息
FUSB302_ret_t FUSB302_get_ID          (FUSB302_dev_t *dev, uint8_t *version_ID, uint8_t *revision_ID);
// 获取cc1 cc2 线上的电压
FUSB302_ret_t FUSB302_get_cc          (FUSB302_dev_t *dev, uint8_t *cc1, uint8_t *cc2);
// 获取VBUS的水平用来判断是否有接口连接
FUSB302_ret_t FUSB302_get_vbus_level  (FUSB302_dev_t *dev, uint8_t *vbus);
// 获取消息 消息头 和消息内容
FUSB302_ret_t FUSB302_get_message     (FUSB302_dev_t *dev, uint16_t *header, uint32_t *data);
// FUSB302发送数据 自动封装
FUSB302_ret_t FUSB302_tx_sop          (FUSB302_dev_t *dev, uint16_t header, const uint32_t *data);
// FUSB302 发送硬复位报文  重置 FUSB302 tx rx 逻辑？
FUSB302_ret_t FUSB302_tx_hard_reset   (FUSB302_dev_t *dev);
// 根据FUSB302的状态 执行相关操作
/*若断开 就会一直监听VBUSOK位 如果为1 表示有连接 这时给内部振荡器供电 并读cc1 cc2的电平 清空中断寄存器 打开应该对应的cc脚 更新状态*/
/*若连接 就读取7个状态寄存器 共处理四种情况 
1 当vbus感知打开且VBUSOK为0时 表示断开 拉低cc引脚且关闭内部振荡器供电 更新状态 直接返回
2 收到硬复位信号 复位相关寄存器 直接返回
3 监听 (发送一个GoodCRC确认报文来响应一个有正确CRC值的入站报文)中断信号 清楚并更新事件
4 监听RX_EMPTY位 如果为0 表示FUSB302接受缓冲区不为空 接受信息*/
FUSB302_ret_t FUSB302_alert           (FUSB302_dev_t *dev, FUSB302_event_t *events);

#endif /* FUSB302_H */

