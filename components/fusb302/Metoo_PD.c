#if 0
#include "Metoo_PD.h"
#include "main.h"
// #include "MetooConfig.h"
// #include "Metoo_I2C.h"

#define FUSB302_FLAG_TIMEOUT ((uint32_t)0x1000)
#define FUSB302_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))

uint32_t I2C_Timeout = FUSB302_FLAG_TIMEOUT;

#define t_PD_POLLING 100
#define t_TypeCSinkWaitCap 350
#define t_RequestToPSReady 580 // combine t_SenderResponse and t_PSTransition
#define t_PPSRequest 5000      // must less than 10000 (10s)

enum
{
    STATUS_LOG_MSG_TX,
    STATUS_LOG_MSG_RX,
    STATUS_LOG_DEV,
    STATUS_LOG_CC,
    STATUS_LOG_SRC_CAP,
    STATUS_LOG_POWER_READY,
    STATUS_LOG_POWER_PPS_STARTUP,
    STATUS_LOG_POWER_REJECT,
    STATUS_LOG_LOAD_SW_ON,
    STATUS_LOG_LOAD_SW_OFF,
};

// Device
FUSB302_dev_t FUSB302;
PD_protocol_t protocol;

// Power ready power
uint16_t ready_voltage;
uint16_t ready_current;
// PPS setup
uint16_t PPS_voltage_next;
uint8_t PPS_current_next;

// 初始化状态位
uint8_t status_initialized;
uint8_t status_src_cap_received;
status_power_t status_power;
// Timer and counter for PD Policy
uint16_t time_polling;
uint16_t time_wait_src_cap;
uint16_t time_wait_ps_rdy;
uint16_t time_PPS_request;
uint8_t get_src_cap_retry_count;
uint8_t wait_src_cap;
uint8_t wait_ps_rdy;
uint8_t send_request;
static uint8_t clock_prescaler = 1;

// status log event queue
status_log_t status_log[16]; // array size must be power of 2 and <=256
uint8_t status_log_read;
uint8_t status_log_write;
// status log object queue
uint32_t status_log_obj[16]; // array size must be power of 2 and <=256
uint8_t status_log_obj_read;
uint8_t status_log_obj_write;
// state variables
pd_log_level_t status_log_level = PD_LOG_LEVEL_VERBOSE;
uint8_t status_log_counter;
char status_log_time[8];

MetooPD_Data_t MetooPD_Data = {
    .state = PD_OFF,
    .setV = 5,
    .PD_V = PD_POWER_OPTION_MAX_5V,
    .pps_c = 2.0,
};

// Init
uint8_t objj[8];

void Metoo_PD_init(void)
{
    FUSB302.i2c_address = FUSB302_I2C_ADDRESS;
    FUSB302.i2c_read = PD_UFP_core_FUSB302_i2c_read;
    FUSB302.i2c_write = PD_UFP_core_FUSB302_i2c_write;
    FUSB302.delay_ms = PD_UFP_core_delay_ms;

    if (FUSB302_init(&FUSB302) == FUSB302_SUCCESS && FUSB302_get_ID(&FUSB302, 0, 0) == FUSB302_SUCCESS)
    {
        status_initialized = 1;
    }
    PD_protocol_init(&protocol);
}

void Metoo_PD_init_TYP(enum PD_power_option_t power_option)
{
    PD_protocol_set_power_option(&protocol, power_option);
    PD_UFP_log_status_log_event(STATUS_LOG_DEV, objj);
}

void Metoo_PD_init_PPS(uint16_t PPS_voltage, uint8_t PPS_current)
{

    PD_protocol_set_PPS(&protocol, PPS_voltage, PPS_current, false);
    PD_UFP_log_status_log_event(STATUS_LOG_DEV, objj);
}

void Metoo_PD_rst(MetooPD_Data_t *PD)
{
    // FUSB302_pd_reset(&FUSB302);
    // PD_protocol_reset(&protocol);
}

__IO uint64_t pd_time0, pd_time1;
#define PD_TIME 30000 // 超时
void Metoo_PD_loop(MetooPD_Data_t *PD)
{
    switch (PD->state)
    {
    case PD_START_1:
        pd_time0 = millis();
        Metoo_PD_init_TYP(PD_POWER_OPTION_MAX_12V);
        PD->state = PD_START_2;
        break;

    case PD_START_2:
        break;

    case PD_ON:
        PD_UFP_core_set_power_option(PD->PD_V);
        break;
    case PD_PPS_START_1:
        pd_time0 = millis();
        Metoo_PD_init_PPS(PPS_V(5.0), PPS_A(3.0));
        PD->state = PD_PPS_START_2;
        break;
    case PD_PPS_START_2:
        break;

    case PD_PPS_ON:
        PD_UFP_core_set_PPS(PPS_V(PD->setV), PPS_A(MetooPD_Data.pps_c));
        break;

    case PD_FAIL:
        PD->state = PD_OFF;
        break;
    default:
        break;
    }
    if (PD->state != PD_OFF)
    {
        PD_UFP_core_run();
        PD_UFP_log_print_status();
    }
}

void PD_UFP_core_run(void)
{
    if (PD_UFP_core_timer() || GPIO_ReadInputDataBit(FUSB302_INT_PIN_PROT, FUSB302_INT_PIN) == 0)
    {
        FUSB302_event_t FUSB302_events = 0;
        for (uint8_t i = 0; i < 3 && FUSB302_alert(&FUSB302, &FUSB302_events) != FUSB302_SUCCESS; i++)
        {
        }
        if (FUSB302_events)
        {
            PD_UFP_core_handle_FUSB302_event(FUSB302_events);
        }
    }
}

bool PD_UFP_core_set_PPS(uint16_t PPS_voltage, uint8_t PPS_current)
{
    if (status_power == STATUS_POWER_PPS && PD_protocol_set_PPS(&protocol, PPS_voltage, PPS_current, true))
    {
        send_request = 1;
        return true;
    }
    return false;
}

void PD_UFP_core_set_power_option(enum PD_power_option_t power_option)
{
    if (PD_protocol_set_power_option(&protocol, power_option))
    {
        send_request = 1;
    }
}

void PD_UFP_core_clock_prescale_set(uint8_t prescaler)
{
    if (prescaler)
    {
        clock_prescaler = prescaler;
    }
}

void FUSB302_TIMEOUT_UserCallback(void)
{
    // log_info("...I2C_ErrCode\r\n");
    /* Block communication and all processes */
    while (1)
    {
    }
}

FUSB302_ret_t PD_UFP_core_FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
    ErrCode_t errcode = Metoo_I2C_Read(FUSB302_I2C, dev_addr, reg_addr, data, count);
    return errcode == 0 ? FUSB302_SUCCESS : FUSB302_ERR_READ_DEVICE;
}
FUSB302_ret_t PD_UFP_core_FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
    ErrCode_t errcode = Metoo_I2C_Write(FUSB302_I2C, dev_addr, reg_addr, data, count);
    return errcode == 0 ? FUSB302_SUCCESS : FUSB302_ERR_READ_DEVICE;
}

FUSB302_ret_t PD_UFP_core_FUSB302_delay_ms(uint32_t t)
{
    Metoo_Delay_ms(t / clock_prescaler);
    return FUSB302_SUCCESS;
}

void PD_UFP_core_handle_protocol_event(PD_protocol_event_t events)
{
    if (events & PD_PROTOCOL_EVENT_SRC_CAP)
    {
        wait_src_cap = 0;
        get_src_cap_retry_count = 0;
        wait_ps_rdy = 1;
        time_wait_ps_rdy = PD_UFP_core_clock_ms();
        PD_UFP_log_status_log_event(STATUS_LOG_SRC_CAP, objj);
    }
    if (events & PD_PROTOCOL_EVENT_REJECT)
    {
        if (wait_ps_rdy)
        {
            wait_ps_rdy = 0;
            PD_UFP_log_status_log_event(STATUS_LOG_POWER_REJECT, objj);
        }
    }
    if (events & PD_PROTOCOL_EVENT_PS_RDY)
    {
        PD_power_info_t p;
        uint8_t i, selected_power = PD_protocol_get_selected_power(&protocol);
        PD_protocol_get_power_info(&protocol, selected_power, &p);
        wait_ps_rdy = 0;
        if (p.type == PD_PDO_TYPE_AUGMENTED_PDO)
        {
            // PPS mode
            FUSB302_set_vbus_sense(&FUSB302, 0);
            if (PPS_voltage_next)
            {
                // Two stage startup for PPS voltage < 5V
                PD_protocol_set_PPS(&protocol, PPS_voltage_next, PPS_current_next, false);
                PPS_voltage_next = 0;
                send_request = 1;
                PD_UFP_log_status_log_event(STATUS_LOG_POWER_PPS_STARTUP, objj);
            }
            else
            {
                time_PPS_request = PD_UFP_core_clock_ms();
                PD_UFP_core_status_power_ready(STATUS_POWER_PPS,
                                               PD_protocol_get_PPS_voltage(&protocol), PD_protocol_get_PPS_current(&protocol));
                PD_UFP_log_status_log_event(STATUS_LOG_POWER_READY, objj);
            }
        }
        else
        {
            FUSB302_set_vbus_sense(&FUSB302, 1);
            PD_UFP_core_status_power_ready(STATUS_POWER_TYP, p.max_v, p.max_i);
            PD_UFP_log_status_log_event(STATUS_LOG_POWER_READY, objj);
        }
    }
}

void PD_UFP_core_handle_FUSB302_event(FUSB302_event_t events)
{
    if (events & FUSB302_EVENT_DETACHED)
    {
        PD_protocol_reset(&protocol);
        return;
    }
    if (events & FUSB302_EVENT_ATTACHED)
    {
        uint8_t cc1 = 0, cc2 = 0, cc = 0;
        FUSB302_get_cc(&FUSB302, &cc1, &cc2);
        PD_protocol_reset(&protocol);
        if (cc1 && cc2 == 0)
        {
            cc = cc1;
        }
        else if (cc2 && cc1 == 0)
        {
            cc = cc2;
        }
        /* TODO: handle no cc detected error */
        if (cc > 1)
        {
            wait_src_cap = 1;
        }
        else
        {
            PD_UFP_core_set_default_power();
        }
        PD_UFP_log_status_log_event(STATUS_LOG_CC, objj);
    }
    if (events & FUSB302_EVENT_RX_SOP)
    {
        PD_protocol_event_t protocol_event = 0;
        uint16_t header;
        uint32_t obj[7];
        FUSB302_get_message(&FUSB302, &header, obj);
        PD_protocol_handle_msg(&protocol, header, obj, &protocol_event);
        PD_UFP_log_status_log_event(STATUS_LOG_MSG_RX, obj);
        if (protocol_event)
        {
            PD_UFP_core_handle_protocol_event(protocol_event);
        }
    }
    if (events & FUSB302_EVENT_GOOD_CRC_SENT)
    {
        uint16_t header;
        uint32_t obj[7];
        PD_UFP_core_delay_ms(2); /* Delay respond in case there are retry messages */
        if (PD_protocol_respond(&protocol, &header, obj))
        {
            PD_UFP_log_status_log_event(STATUS_LOG_MSG_TX, obj);
            FUSB302_tx_sop(&FUSB302, header, obj);
        }
    }
}

bool PD_UFP_core_timer(void)
{
    uint16_t t = PD_UFP_core_clock_ms();
    if (wait_src_cap && t - time_wait_src_cap > t_TypeCSinkWaitCap)
    {
        time_wait_src_cap = t;
        if (get_src_cap_retry_count < 3)
        {
            uint16_t header;
            get_src_cap_retry_count += 1;
            /* Try to request soruce capabilities message (will not cause power cycle VBUS) */
            PD_protocol_create_get_src_cap(&protocol, &header);
            PD_UFP_log_status_log_event(STATUS_LOG_MSG_TX, objj);
            FUSB302_tx_sop(&FUSB302, header, 0);
        }
        else
        {
            get_src_cap_retry_count = 0;
            /* Hard reset will cause the source power cycle VBUS. */
            FUSB302_tx_hard_reset(&FUSB302);
            PD_protocol_reset(&protocol);
        }
    }
    if (wait_ps_rdy)
    {
        if (t - time_wait_ps_rdy > t_RequestToPSReady)
        {
            wait_ps_rdy = 0;
            PD_UFP_core_set_default_power();
        }
    }
    else if (send_request || (status_power == STATUS_POWER_PPS && t - time_PPS_request > t_PPSRequest))
    {
        wait_ps_rdy = 1;
        send_request = 0;
        time_PPS_request = t;
        uint16_t header;
        uint32_t obj[7];
        /* Send request if option updated or regularly in PPS mode to keep power alive */
        PD_protocol_create_request(&protocol, &header, obj);
        PD_UFP_log_status_log_event(STATUS_LOG_MSG_TX, obj);
        time_wait_ps_rdy = PD_UFP_core_clock_ms();
        FUSB302_tx_sop(&FUSB302, header, obj);
    }
    if (t - time_polling > t_PD_POLLING)
    {
        time_polling = t;
        return true;
    }
    return false;
}

void PD_UFP_core_set_default_power(void)
{
    PD_UFP_core_status_power_ready(STATUS_POWER_TYP, PD_V(5), PD_A(1));
    PD_UFP_log_status_log_event(STATUS_LOG_POWER_READY, objj);
}

void PD_UFP_core_status_power_ready(status_power_t status, uint16_t voltage, uint16_t current)
{
    ready_voltage = voltage;
    ready_current = current;
    status_power = status;
}

uint8_t PD_UFP_core_clock_prescaler = 1;

void PD_UFP_core_delay_ms(uint16_t ms)
{
    Metoo_Delay_ms(ms / clock_prescaler);
}

uint16_t PD_UFP_core_clock_ms(void)
{
    return (uint16_t)millis() * clock_prescaler;
}

bool is_power_ready(void) { return status_power == STATUS_POWER_TYP; }
bool is_PPS_ready(void) { return status_power == STATUS_POWER_PPS; }
bool is_ps_transition(void) { return send_request || wait_ps_rdy; }

uint16_t get_voltage(void) { return ready_voltage; } // Voltage in 50mV units, 20mV(PPS)
uint16_t get_current(void) { return ready_current; } // Current in 10mA units, 50mA(PPS)


///////////////////////////////////////////////////////////////////////////////////////////////////
// Optional: PD_UFP_log_c, extended from PD_UFP_c to provide logging function.
//           Asynchronous, minimal impact on PD timing.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define STATUS_LOG_MASK (sizeof(status_log) / sizeof(status_log[0]) - 1)
#define STATUS_LOG_OBJ_MASK (sizeof(status_log_obj) / sizeof(status_log_obj[0]) - 1)

#define SNPRINTF snprintf
#define PSTR(str) str

uint8_t PD_UFP_log_status_log_obj_add(uint16_t header, uint32_t *obj)
{
    if (obj)
    {
        uint8_t i, w = status_log_obj_write, r = status_log_obj_read;
        PD_msg_info_t info;
        PD_protocol_get_msg_info(header, &info);
        for (i = 0; i < info.num_of_obj && (uint8_t)(w - r) < STATUS_LOG_OBJ_MASK; i++)
        {
            status_log_obj[w++ & STATUS_LOG_OBJ_MASK] = obj[i];
        }
        status_log_obj_write = w;
        return i;
    }
    return 0;
}

void PD_UFP_log_status_log_event(uint8_t status, uint32_t *obj)
{
    if (((status_log_write - status_log_read) & STATUS_LOG_MASK) >= STATUS_LOG_MASK)
    {
        return;
    }
    status_log_t *log = &status_log[status_log_write & STATUS_LOG_MASK];
    switch (status)
    {
    case STATUS_LOG_MSG_TX:
        log->msg_header = PD_protocol_get_tx_msg_header(&protocol);
        log->obj_count = PD_UFP_log_status_log_obj_add(log->msg_header, obj);
        break;
    case STATUS_LOG_MSG_RX:
        log->msg_header = PD_protocol_get_rx_msg_header(&protocol);
        log->obj_count = PD_UFP_log_status_log_obj_add(log->msg_header, obj);
        break;
    default:
        break;
    }
    log->status = status;
    log->time = PD_UFP_core_clock_ms();
    status_log_write++;
}

int PD_UFP_log_status_log_readline_msg(char *buffer, int maxlen, status_log_t *log)
{
    char *t = status_log_time;
    int n = 0;
    if (status_log_counter == 0)
    {
        // output message header
        char type = log->status == STATUS_LOG_MSG_TX ? 'T' : 'R';
        PD_msg_info_t info;
        PD_protocol_get_msg_info(log->msg_header, &info);
        if (status_log_level >= PD_LOG_LEVEL_VERBOSE)
        {
            const char *ext = info.extended ? "ext, " : "";
            // log_info("%s%cX %s id=%d %sraw=0x%04X\n", t, type, info.name, info.id, ext, log->msg_header);
            if (info.num_of_obj)
            {
                status_log_counter++;
            }
        }
        else
        {
            // log_info("%s%cX %s\n", t, type, info.name);
        }
    }
    else
    {
        // output object data
        int i = status_log_counter - 1;
        uint32_t obj = status_log_obj[status_log_obj_read++ & STATUS_LOG_OBJ_MASK];
        // log_info("%s obj%d=0x%08lX\n", t, i, obj);
        if (++status_log_counter > log->obj_count)
        {
            status_log_counter = 0;
        }
    }
    return n;
}

int PD_UFP_log_status_log_readline_src_cap(char *buffer, int maxlen)
{
    PD_power_info_t p;
    int n = 0;
    uint8_t i = status_log_counter;
    if (PD_protocol_get_power_info(&protocol, i, &p))
    {
        const char *str_pps[] = {"", " BAT", " VAR", " PPS"}; /* PD_power_data_obj_type_t */
        char *t = status_log_time;
        uint8_t selected = PD_protocol_get_selected_power(&protocol);
        char min_v[10] = {0}, max_v[10] = {0}, power[10] = {0};
        if (p.min_v)
            SNPRINTF(min_v, sizeof(min_v) - 1, PSTR("%2d.%02dV-"), p.min_v / 20, (p.min_v * 5) % 100);
        if (p.max_v)
            SNPRINTF(max_v, sizeof(max_v) - 1, PSTR("%2d.%02dV"), p.max_v / 20, (p.max_v * 5) % 100);
        if (p.max_i)
            SNPRINTF(power, sizeof(power) - 1, PSTR(" %2d.%02dA"), p.max_i / 100, p.max_i % 100);
        else
            SNPRINTF(power, sizeof(power) - 1, PSTR(" %2d.%02dW"), p.max_p / 4, p.max_p * 25);

        // log_info("%s   [%d] %s%s %s%s%s\n", t, i, min_v, max_v, power, str_pps[p.type], i == selected ? " *" : ""); // 输出详细列表

        if (MetooPD_Data.PD_info_state == PDO_START1)
        {
            if (p.type == PD_PDO_TYPE_AUGMENTED_PDO)
                MetooPD_Data.pps_c = p.max_i / 100; // PPS最大电流
            strcat(MetooPD_Data.PD_info, min_v);
            strcat(MetooPD_Data.PD_info, max_v);
            strcat(MetooPD_Data.PD_info, power);
            strcat(MetooPD_Data.PD_info, str_pps[p.type]);
            strcat(MetooPD_Data.PD_info, "\n");
        }

        status_log_counter++;
    }
    else
    {
        MetooPD_Data.PD_info_state = PDO_START2;
        // log_info("%s", MetooPD_Data.PD_info);
        status_log_counter = 0;
    }
    return n;
}

int PD_UFP_log_status_log_readline(char *buffer, int maxlen)
{
    if (status_log_write == status_log_read)
    {
        return 0;
    }

    status_log_t *log = &status_log[status_log_read & STATUS_LOG_MASK];
    int n = 0;
    char *t = status_log_time;
    if (t[0] == 0)
    { // Convert timestamp number to string
        SNPRINTF(t, sizeof(status_log_time) - 1, PSTR("%04u: "), log->time);
        return 0;
    }

    switch (log->status)
    {
    case STATUS_LOG_MSG_TX:
    case STATUS_LOG_MSG_RX:
        n = PD_UFP_log_status_log_readline_msg(buffer, maxlen, log);
        break;
    case STATUS_LOG_DEV:
        if (status_initialized)
        {
            uint8_t version_ID = 0, revision_ID = 0;
            FUSB302_get_ID(&FUSB302, &version_ID, &revision_ID);
            // log_info("\n%sFUSB302 ver ID:%c_rev%c\n", t, 'A' + version_ID, 'A' + revision_ID);
        }
        else
        {
            // log_info("\n%sFUSB302 init error\n", t);
        }
        break;
    case STATUS_LOG_CC:
    {
        const char *detection_type_str[] = {"USB", "1.5", "3.0"};
        uint8_t cc1 = 0, cc2 = 0;
        FUSB302_get_cc(&FUSB302, &cc1, &cc2);
        if (cc1 == 0 && cc2 == 0)
        {
            // log_info("%sUSB attached vRA\n", t);
        }
        else if (cc1 && cc2 == 0)
        {
            // log_info("%sUSB attached CC1 vRd-%s\n", t, detection_type_str[cc1 - 1]);
        }
        else if (cc2 && cc1 == 0)
        {
            // log_info("%sUSB attached CC2 vRd-%s\n", t, detection_type_str[cc2 - 1]);
        }
        else
        {
            // log_info("%sUSB attached unknown\n", t);
        }
        break;
    }
    case STATUS_LOG_SRC_CAP:
        n = PD_UFP_log_status_log_readline_src_cap(buffer, maxlen);
        break;
    case STATUS_LOG_POWER_READY:
    {
        uint16_t v = ready_voltage;
        uint16_t a = ready_current;
        if (status_power == STATUS_POWER_TYP)
        {
            // log_info("%s%d.%02dV %d.%02dA supply ready\n", t, v / 20, (v * 5) % 100, a / 100, a % 100);
            if (MetooPD_Data.PD_info_state)
            {
                if (MetooPD_Data.state == PD_START_2)
                {
                    MetooPD_Data.state = PD_ON;
                    Metoo_S4_set();
                    Metoo_S5_MSG(true);
                }
                else
                {
                    MetooPD_Data.state = PD_FAIL;
                    Metoo_S5_MSG(false);
                }
                MetooPD_Data.PD_info_state = PDO_END;
                Metoo_S4_MSG(true);
            }
        }
        else if (status_power == STATUS_POWER_PPS)
        {
            // log_info("%sPPS %d.%02dV %d.%02dA supply ready\n", t, v / 50, (v * 2) % 100, a / 20, (a * 5) % 100);
            if (MetooPD_Data.PD_info_state)
            {
                if (MetooPD_Data.state == PD_PPS_START_2)
                {
                    MetooPD_Data.state = PD_PPS_ON;
                    Metoo_S5_MSG(true);
                }
                else
                {
                    MetooPD_Data.state = PD_FAIL;
                    Metoo_S5_MSG(false);
                }
                MetooPD_Data.PD_info_state = PDO_END;
            }
        }
        // log_info("MetooPD_Data.state = %d\n", MetooPD_Data.state);
        break;
    }
    case STATUS_LOG_POWER_PPS_STARTUP:
        // log_info("%sPPS 2-stage startup\n", t);
        break;
    case STATUS_LOG_POWER_REJECT:
        // log_info("%sRequest Rejected\n", t);
        break;
    case STATUS_LOG_LOAD_SW_ON:
        // log_info("%sLoad SW ON\n", t);
        break;
    case STATUS_LOG_LOAD_SW_OFF:
        // log_info("%sLoad SW OFF\n", t);
        break;
    }
    if (status_log_counter == 0)
    {
        t[0] = 0;
        status_log_read++;
        status_log_counter = 0;
    }
    return n;
}

void PD_UFP_log_print_status(void)
{
    // Wait for enough tx buffer in serial port to avoid blocking

    char buf[64];
    if (PD_UFP_log_status_log_readline(buf, sizeof(buf) - 1))
    {
        printf(buf);
    }
}
#endif