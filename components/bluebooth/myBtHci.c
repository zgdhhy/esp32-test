#include "myBtHci.h"

void controller_rcv_pkt_ready(void)
{
    printf("controller rcv pkt ready\n");
}


int host_rcv_pkt(uint8_t *data, uint16_t len)
{
    printf("host rcv pkt: ");
    for (uint16_t i = 0; i < len; i++) {
        printf("0x%02x ", data[i]);
    }
    printf("\n");
    return 0;
}

uint16_t make_cmd_ble_set_adv_enable (uint8_t *buf, uint8_t adv_enable)
{
    UINT8_2_BUFFER (buf, H4_TYPE_COMMAND);
    UINT16_2_BUFFER (buf, HCI_BLE_WRITE_ADV_ENABLE);
    UINT8_2_BUFFER (buf, HCIC_PARAM_SIZE_WRITE_ADV_ENABLE);
    UINT8_2_BUFFER (buf, adv_enable);
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_WRITE_ADV_ENABLE;
}

uint16_t make_cmd_ble_set_adv_param (uint8_t *buf, uint16_t adv_int_min, uint16_t adv_int_max,
                                     uint8_t adv_type, uint8_t addr_type_own,
                                     uint8_t addr_type_dir, bd_addr_t direct_bda,
                                     uint8_t channel_map, uint8_t adv_filter_policy)
{
    UINT8_2_BUFFER (buf, H4_TYPE_COMMAND);
    UINT16_2_BUFFER (buf, HCI_BLE_WRITE_ADV_PARAMS);
    UINT8_2_BUFFER  (buf, HCIC_PARAM_SIZE_BLE_WRITE_ADV_PARAMS );

    UINT16_2_BUFFER (buf, adv_int_min);
    UINT16_2_BUFFER (buf, adv_int_max);
    UINT8_2_BUFFER (buf, adv_type);
    UINT8_2_BUFFER (buf, addr_type_own);
    UINT8_2_BUFFER (buf, addr_type_dir);
    BDADDR_TO_STREAM (buf, direct_bda);
    UINT8_2_BUFFER (buf, channel_map);
    UINT8_2_BUFFER (buf, adv_filter_policy);
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_BLE_WRITE_ADV_PARAMS;
}

uint16_t make_cmd_reset(uint8_t *buf)
{
    UINT8_2_BUFFER (buf, H4_TYPE_COMMAND);
    UINT16_2_BUFFER (buf, HCI_RESET);
    UINT8_2_BUFFER (buf, 0);
    return HCI_H4_CMD_PREAMBLE_SIZE;
}



uint16_t make_cmd_ble_set_adv_data(uint8_t *buf, uint8_t data_len, uint8_t *p_data)
{
    UINT8_2_BUFFER (buf, H4_TYPE_COMMAND);
    UINT16_2_BUFFER (buf, HCI_BLE_WRITE_ADV_DATA);
    UINT8_2_BUFFER  (buf, HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA + 1);

    memset(buf, 0, HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA);

    if (p_data != NULL && data_len > 0) {
        if (data_len > HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA) {
            data_len = HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA;
        }

        UINT8_2_BUFFER (buf, data_len);

        ARRAY_2_BUFFER (buf, p_data, data_len);
    }
    return HCI_H4_CMD_PREAMBLE_SIZE + HCIC_PARAM_SIZE_BLE_WRITE_ADV_DATA + 1;
}

void hci_cmd_send_reset(void)
{
    uint16_t sz = make_cmd_reset (hci_cmd_buf);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

void hci_cmd_send_ble_adv_start(void)
{
    uint16_t sz = make_cmd_ble_set_adv_enable (hci_cmd_buf, 1);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

void hci_cmd_send_ble_set_adv_param(void)
{
    uint16_t adv_intv_min = 256; // 160ms
    uint16_t adv_intv_max = 256; // 160ms
    uint8_t adv_type = 0; // connectable undirected advertising (ADV_IND)
    uint8_t own_addr_type = 0; // Public Device Address
    uint8_t peer_addr_type = 0; // Public Device Address
    uint8_t peer_addr[6] = {0};
    uint8_t adv_chn_map = 0x07; // 37, 38, 39
    uint8_t adv_filter_policy = 0; // Process All Conn and Scan

    uint16_t sz = make_cmd_ble_set_adv_param(hci_cmd_buf,
                                             adv_intv_min,
                                             adv_intv_max,
                                             adv_type,
                                             own_addr_type,
                                             peer_addr_type,
                                             peer_addr,
                                             adv_chn_map,
                                             adv_filter_policy);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

void hci_cmd_send_ble_set_adv_data(void)
{
    char *adv_name = "MYBT ESP32";
    uint8_t name_len = (uint8_t)strlen(adv_name);
    uint8_t adv_data[31] = {0x02, 0x01, 0x06, 0x0, 0x09};
    uint8_t adv_data_len;

    adv_data[3] = name_len + 1;
    for (int i = 0; i < name_len; i++) {
        adv_data[5 + i] = (uint8_t)adv_name[i];
    }
    adv_data_len = 5 + name_len;

    uint16_t sz = make_cmd_ble_set_adv_data(hci_cmd_buf, adv_data_len, (uint8_t *)adv_data);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}