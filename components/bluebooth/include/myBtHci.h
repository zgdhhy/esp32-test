#ifndef MYBTHCI_H
#define MYBTHCI_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint8_t hci_cmd_buf[128];
#define BD_ADDR_LEN     (6)                     /* Device address length */
typedef uint8_t bd_addr_t[BD_ADDR_LEN];         /* Device address */

#define BDADDR_TO_STREAM(p, a)   {int ijk; for (ijk = 0; ijk < BD_ADDR_LEN;  ijk++) *(p)++ = (uint8_t) a[BD_ADDR_LEN - 1 - ijk];}

#define HCI_H4_CMD_PREAMBLE_SIZE           (4)

/*  HCI Command Opcode group Field (OGF) */
#define HCI_GRP_HOST_CONT_BASEBAND_CMDS    (0x03 << 10)            /* 0x0C00 */
#define HCI_GRP_BLE_CMDS                   (0x08 << 10)

/*  HCI Command Opcode Command Field (OCF) */
#define HCI_RESET                       (0x0003 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
/* Advertising Commands. */
#define HCI_BLE_WRITE_ADV_ENABLE        (0x000A | HCI_GRP_BLE_CMDS)
#define HCI_BLE_WRITE_ADV_DATA          (0x0008 | HCI_GRP_BLE_CMDS)
#define HCI_BLE_WRITE_ADV_PARAMS        (0x0006 | HCI_GRP_BLE_CMDS)

/* HCI Command length. */
#define HCIC_PARAM_SIZE_WRITE_ADV_ENABLE        1
#define HCIC_PARAM_SIZE_BLE_WRITE_ADV_PARAMS    15
#define HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA      31

enum {
    H4_TYPE_COMMAND = 1,
    H4_TYPE_ACL     = 2,
    H4_TYPE_SCO     = 3,
    H4_TYPE_EVENT   = 4
};
void controller_rcv_pkt_ready(void);
int host_rcv_pkt(uint8_t *data, uint16_t len);
void hci_cmd_send_reset(void);
void hci_cmd_send_ble_adv_start(void);
void hci_cmd_send_ble_set_adv_param(void);
void hci_cmd_send_ble_set_adv_data(void);
#endif
