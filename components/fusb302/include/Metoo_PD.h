#if 0
#ifndef METOO_PD_H_
#define METOO_PD_H_

#include "main.h"
#include "FUSB302_UFP.h"
#include "PD_UFP_Protocol.h"

#define FUSB302_I2C I2C2
#define FUSB302_I2C_PORT GPIOB
#define FUSB302_I2C_PER_CLOCK RCC_APB1_PERIPH_I2C2
#define FUSB302_I2C_GPIO_CLOCK RCC_APB2_PERIPH_AFIO | RCC_APB2_PERIPH_GPIOB
#define FUSB302_I2C_SDA_PIN GPIO_PIN_11
#define FUSB302_I2C_SCL_PIN GPIO_PIN_10

#define FUSB302_INT_PIN_PROT GPIOB
#define FUSB302_INT_PIN GPIO_PIN_8

#define FUSB302_I2C_CLOCK_SPEED 100000
#define FUSB302_I2C_ADDRESS (0x22 << 1)
typedef enum
{
    STATUS_POWER_NA = 0,
    STATUS_POWER_TYP,
    STATUS_POWER_PPS
} status_power_t;

typedef enum
{
    PD_OFF,

    PD_START_1, // 启动阶段
    PD_START_2, // 启动阶段(已经init)
    PD_ON,
    PD_SET,

    PD_PPS_START_1, // 启动PPS
    PD_PPS_START_2,
    PD_PPS_ON,
    PD_PPS_SET,

    PD_FAIL,
    PD_RST,
    PD_CHECK, // PD 检测
} MetooPD_State_t;

typedef enum
{
    PDO_END,
    PDO_START1,
    PDO_START2,
} MetooPD_PDO_State_t;  

typedef struct
{
    MetooPD_State_t state;
    float setV;
    float pps_c;
    enum PD_power_option_t PD_V;
    char PD_info[128];
    MetooPD_PDO_State_t PD_info_state;
} MetooPD_Data_t;

extern MetooPD_Data_t MetooPD_Data;

typedef struct
{
    uint16_t time;
    uint16_t msg_header;
    uint8_t obj_count;
    uint8_t status;
} status_log_t;
typedef enum
{
    PD_LOG_LEVEL_INFO,
    PD_LOG_LEVEL_VERBOSE
} pd_log_level_t;

// Init
// pd 初始化 1 给FUSB302赋值 从机地址 I2C读写函数 延时函数
void Metoo_PD_init(void);
// 设置电源选项
void Metoo_PD_init_TYP(enum PD_power_option_t power_option);
// PPS电压设置为20mV单位，电流设置为50mA单位 如果PPS设置不合格，则退回到常规电源选项
void Metoo_PD_init_PPS(uint16_t PPS_voltage, uint8_t PPS_current);
// 复位 (舍去了)
void Metoo_PD_rst(MetooPD_Data_t *PD);
// Task
void PD_UFP_core_run(void);
void Metoo_PD_loop(MetooPD_Data_t *PD);
// Status
bool is_power_ready(void);
bool is_PPS_ready(void);
bool is_ps_transition(void);
// Get
uint16_t get_voltage(void);
uint16_t get_current(void);
// Set
bool PD_UFP_core_set_PPS(uint16_t PPS_voltage, uint8_t PPS_current);
void PD_UFP_core_set_power_option(enum PD_power_option_t power_option);
// Clock
void PD_UFP_core_clock_prescale_set(uint8_t prescaler);

FUSB302_ret_t PD_UFP_core_FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
FUSB302_ret_t PD_UFP_core_FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);

void PD_UFP_core_handle_protocol_event(PD_protocol_event_t events);
void PD_UFP_core_handle_FUSB302_event(FUSB302_event_t events);

bool PD_UFP_core_timer(void);

void PD_UFP_core_set_default_power(void);
void PD_UFP_core_status_power_ready(status_power_t status, uint16_t voltage, uint16_t current);

void PD_UFP_core_delay_ms(uint16_t ms);
uint16_t PD_UFP_core_clock_ms(void);
// log
uint8_t PD_UFP_log_status_log_obj_add(uint16_t header, uint32_t *obj);
void PD_UFP_log_status_log_event(uint8_t status, uint32_t *obj);
int PD_UFP_log_status_log_readline_msg(char *buffer, int maxlen, status_log_t *log);
int PD_UFP_log_status_log_readline_src_cap(char *buffer, int maxlen);
int PD_UFP_log_status_log_readline(char *buffer, int maxlen);
void PD_UFP_log_print_status(void);
#endif /* METOO_PD_H_ */
#endif /* METOO_PD_H_ */