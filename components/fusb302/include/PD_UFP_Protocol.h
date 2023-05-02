/**
 * PD_UFP_Protocol.c
 *
 *  Updated on: Aug 25, 2021
 *      Author: Ryan Ma
 *
 * Minimalist USB PD implement with only UFP(device) sink only functionality
 * Requires PD PHY to do automatic GoodCRC response on valid SOP messages.
 * Requires only stdint.h, stdbool.h and string.h
 * No use of bit-field for better cross-platform compatibility
 *
 * Support PD3.0 PPS
 * Do not support extended message. Not necessary for PD trigger and PPS.
 * 
 * Reference: USB_PD_R2_0 V1.3 - 20170112
 *            USB_PD_R3_0 V2.0 20190829 + ECNs 2020-12-10
 *            - Chapter 6. Protocol Layer
 *
 */

#ifndef PD_UFP_PROTOCOL_H
#define PD_UFP_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

/* For use in PD_protocol_get_power_info() */
#define PD_V(v)     ((uint16_t)(v * 20 + 0.01))
#define PD_A(a)     ((uint16_t)(a * 100 + 0.01))

/* For use in PD_protocol_set_PPS_option() */
#define PPS_V(v)    ((uint16_t)(v * 50 + 0.01))
#define PPS_A(a)    ((uint8_t)(a * 20 + 0.01))

#define PD_PROTOCOL_MAX_NUM_OF_PDO      7

#define PD_PROTOCOL_EVENT_SRC_CAP       (1 << 0)    // 获取源电源能力事件
#define PD_PROTOCOL_EVENT_PS_RDY        (1 << 1)    // 源电源准备好事件
#define PD_PROTOCOL_EVENT_ACCEPT        (1 << 2)    // 同意事件
#define PD_PROTOCOL_EVENT_REJECT        (1 << 3)    // 拒绝事件
#define PD_PROTOCOL_EVENT_PPS_STATUS    (1 << 4)

typedef uint8_t PD_protocol_event_t;

enum PD_power_option_t {
    PD_POWER_OPTION_MAX_5V      = 0,
    PD_POWER_OPTION_MAX_9V      = 1,
    PD_POWER_OPTION_MAX_12V     = 2,
    PD_POWER_OPTION_MAX_15V     = 3,
    PD_POWER_OPTION_MAX_20V     = 4,
    PD_POWER_OPTION_MAX_VOLTAGE = 5,
    PD_POWER_OPTION_MAX_CURRENT = 6,
    PD_POWER_OPTION_MAX_POWER   = 7,
};

// 电源类型
enum PD_power_data_obj_type_t {   /* Power data object type */
    PD_PDO_TYPE_FIXED_SUPPLY    = 0,    // 固定电源
    PD_PDO_TYPE_BATTERY         = 1,    // 电池
    PD_PDO_TYPE_VARIABLE_SUPPLY = 2,    // 可变电源
    PD_PDO_TYPE_AUGMENTED_PDO   = 3     // 可编程电源 /* USB PD 3.0 */
};

enum PPS_PTF_t {
    PPS_PTF_NOT_SUPPORT         = 0,
    PPS_PTF_NORMAL              = 1,
    PPS_PTF_WARNING             = 2,
    PPS_PTF_OVER_TEMPERATURE    = 3
};

enum PPS_OMF_t {
    PPS_OMF_VOLTAGE_MODE        = 0,
    PPS_OMF_CURRENT_LIMIT_MODE  = 1
};

// 可编程电源状态
typedef struct {
    // 输出电压
    uint16_t output_voltage;    /* Voltage in 20mV units, 0xFFFF if not supported */
    // 输出电流
    uint8_t output_current;     /* Current in 50mV units, 0xFF if not supported */
    enum PPS_PTF_t flag_PTF;
    enum PPS_OMF_t flag_OMF;
} PPS_status_t;

// 消息头
typedef struct {
    const char * name;  // 消息名称
    uint8_t id;         // 消息id
    uint8_t spec_rev;   // 此消息的当前版本
    uint8_t num_of_obj; // 数据个数
    uint8_t extended;   // 是否为扩展信息
} PD_msg_info_t;

// 电源输出功率信息
typedef struct {
    enum PD_power_data_obj_type_t type;
    uint16_t min_v;     /* Voltage in 50mV units */
    uint16_t max_v;     /* Voltage in 50mV units */
    uint16_t max_i;     /* Current in 10mA units */
    uint16_t max_p;     /* Power in 250mW units */
} PD_power_info_t;

struct PD_msg_state_t;
typedef struct {
    const struct PD_msg_state_t *msg_state;                 // 信息处理 包括信息的名字 处理函数 和响应函数
    uint16_t tx_msg_header;                                 // 发送信息头
    uint16_t rx_msg_header;                                 // 接受信息头
    uint8_t message_id;                                     // 信息id

    uint16_t PPS_voltage;                                   // 编程电源电压
    uint8_t PPS_current;                                    // 编程电源电流
    uint8_t PPSSDB[4];  /* PPS Status Data Block */         // PPS状态数据块 

    enum PD_power_option_t power_option;                    // 固定电源选项
    uint32_t power_data_obj[PD_PROTOCOL_MAX_NUM_OF_PDO];    // 源电源供电能力缓存区
    uint8_t power_data_obj_count;                           // 供电能力的个数
    uint8_t power_data_obj_selected;                        // 选择电源能力所在数组的索引
} PD_protocol_t;

/* Message handler */
/*对接受到的信息处理 
1 解析消息头 判断是控制信息 数据信息还是扩展信息
2 如果该消息有定义的处理函数 则调用*/
void PD_protocol_handle_msg(PD_protocol_t *p, uint16_t header, uint32_t *obj, PD_protocol_event_t *events);
// 调用信息对应的响应函数
// 三个参数都不能为空 且p对应的信息处理 和信息处理对应的responder不能为空
bool PD_protocol_respond(PD_protocol_t *p, uint16_t *h, uint32_t *obj);

/* PD Message creation */
// 生成一个想要获取源电压能力的消息头
void PD_protocol_create_get_src_cap(PD_protocol_t *p, uint16_t *header);
// 生成一个想要获取可编程电源状态的消息头
void PD_protocol_create_get_PPS_status(PD_protocol_t *p, uint16_t *header);
// 生成一个请求 根据选择电源能力所在数组的索引 解析出一个电源信息 然后根据电源类型 生成一个请求信息头和信息体
void PD_protocol_create_request(PD_protocol_t *p, uint16_t *header, uint32_t *obj);

/* Get functions */
// 获得要选择电源能力所在数组的索引
static inline uint8_t  PD_protocol_get_selected_power(PD_protocol_t *p) { return p->power_data_obj_selected; }
// 获得编程电源电压
static inline uint16_t PD_protocol_get_PPS_voltage(PD_protocol_t *p) { return p->PPS_voltage; } /* Voltage in 20mV units */
// 获得编程电源电流
static inline uint8_t  PD_protocol_get_PPS_current(PD_protocol_t *p) { return p->PPS_current; } /* Current in 50mA units */
// 获得发送信息头
static inline uint16_t PD_protocol_get_tx_msg_header(PD_protocol_t *p) { return p->tx_msg_header; }
// 获得接受信息头
static inline uint16_t PD_protocol_get_rx_msg_header(PD_protocol_t *p) { return p->rx_msg_header; }

// 根据信息头 解析出对应的信息内容
bool PD_protocol_get_msg_info(uint16_t header, PD_msg_info_t * msg_info);
// 根据index 解析出对应的电源内容
bool PD_protocol_get_power_info(PD_protocol_t *p, uint8_t index, PD_power_info_t *power_info);
// 解析可编程电源状态
bool PD_protocol_get_PPS_status(PD_protocol_t *p, PPS_status_t * PPS_status);

/* Set Fixed and Variable power option */
// 设置电源选项 如果使用的选项不可靠 将默认选择第一个(5V) 并更新适合的电源能力所在数组的索引
bool PD_protocol_set_power_option(PD_protocol_t *p, enum PD_power_option_t option);
// 设置选择电源能力所在数组的索引
bool PD_protocol_select_power(PD_protocol_t *p, uint8_t index);

/* Set PPS Voltage in 20mV units, Current in 50mA units. return true if re-send request is needed
   strict=true, If PPS setting is not qualified, return false, nothing is changed.
   strict=false, if PPS setting is not qualified, fall back to regular power option */
bool PD_protocol_set_PPS(PD_protocol_t * p, uint16_t PPS_voltage, uint8_t PPS_current, bool strict);  

// 复位pd协议 信息处理设置为保留位 并将信息id设为0
void PD_protocol_reset(PD_protocol_t *p);
// 初始化pd协议 分配内存 信息处理设置为保留位
void PD_protocol_init(PD_protocol_t *p);

#endif
