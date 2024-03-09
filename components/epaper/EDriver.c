#include "Edriver.h"

/**
 * @brief EPD所有硬件进行初始化
 * */
void EPD_ALL_Init(void)
{
    EPD_GPIO_Init();
    EPD_SPI_Init();
    EPD_Init();
    EPD_Clear();
}
/**
 * @brief EPD睡眠
 * */
void EPD_Sleep(void)
{
    EPD_SendCommand(0x02); // POWER_OFF
    EPD_ReadBusy();
    EPD_SendCommand(0x07); // DEEP_SLEEP
    EPD_SendData(0xA5); // check code
}

#ifdef EPD_2IN9
/**
 * @brief 判断是否忙
 * */
void EPD_ReadBusy(void)
{
    uint8_t busy;
    do
    {
        EPD_SendCommand(0x71);
        busy = gpio_get_level(EPD_BUSY_PIN);
        busy = !(busy & 0x01);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while (busy);
}
/**
 * @brief 对EDP初始化
 * */
void EPD_Init(void)
{
    EPD_Reset();

    EPD_SendCommand(0x04);
    EPD_ReadBusy();//waiting for the electronic paper IC to release the idle signal

    EPD_SendCommand(0x00);//panel setting
    EPD_SendData(0x1f);//LUT from OTP，128x296
    //    EPD_SendData(0x89);//Temperature sensor, boost and other related timing settings

    EPD_SendCommand(0x61);//resolution setting
    EPD_SendData(0x80);
    EPD_SendData(0x01);
    EPD_SendData(0x28);

    EPD_SendCommand(0X50);//VCOM AND DATA INTERVAL SETTING
    EPD_SendData(0x17);//WBmode:VBDF 17|D7 VBDW 97 VBDB 57
    //WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}
/**
 * @brief 清屏函数 全白
 * */
void EPD_Clear(void)
{
    uint16_t Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
    uint16_t Height = EPD_HEIGHT;
    uint16_t size = Width * Height;
    uint8_t *buf = malloc(size * sizeof(uint8_t));
    for (uint16_t i = 0; i < size; i++) buf[i] = 0x00;
    EPD_SendCommand(0x13);
    EPD_SendDatas(buf, size);
    EPD_SendCommand(0x12);
    EPD_ReadBusy();
    free(buf);
}
/**
 * @brief 写图像 黑色
 * */
void EPD_Display(const uint8_t *blackimage)
{
    uint16_t Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
    uint16_t Height = EPD_HEIGHT;
    EPD_SendCommand(0x13);
    EPD_SendDatas(blackimage, Width * Height);
    EPD_SendCommand(0x12);
    EPD_ReadBusy();
}
/**
 * @brief 写图像 黑色
 * */
void EPD_Partial_Display(uint16_t x, uint16_t y, const uint8_t *image, uint16_t PART_COLUMN, uint16_t PART_PAGE)
{
    if (NULL == image) return;
    uint16_t x_start, x_end, y_start1, y_end1, y_start2, y_end2;
    x_start = x;
    x_end = x + PART_COLUMN - 1;
    if (x_end > EPD_WIDTH - 1) return;
    y_start1 = y / 256;
    y_start2 = y % 256;
    y_end1 = y + PART_PAGE - 1;
    if (y_end1 > EPD_HEIGHT - 1) return;
    y_end2 = y_end1 % 256;
    y_end1 = y_end1 / 256;
    EPD_SendCommand(0x91);
    EPD_SendCommand(0x90);
    EPD_SendData(x_start);
    EPD_SendData(x_end);
    EPD_SendData(y_start1);
    EPD_SendData(y_start2);
    EPD_SendData(y_end1);
    EPD_SendData(y_end2);
    EPD_SendCommand(0x13);
    EPD_SendDatas(image, PART_COLUMN * PART_PAGE);
    EPD_SendCommand(0x12);
    EPD_SendCommand(0x92);
    EPD_ReadBusy();
}
#endif

#ifdef EPD_2IN66
/**
 * @brief 判断是否忙
 * */
void EPD_ReadBusy(void)
{
    while (gpio_get_level(EPD_BUSY_PIN) == 0)
        vTaskDelay(10 / portTICK_PERIOD_MS);
}
/**
 * @brief 对EDP初始化
 * */
void EPD_Init(void)
{
    EPD_Reset();

    EPD_ReadBusy();

    EPD_SendCommand(0x00);
    EPD_SendData(0x0E);
    EPD_ReadBusy();

    EPD_SendCommand(0xE5);
    EPD_SendData(0x19);
    EPD_SendCommand(0xE0);
    EPD_SendData(0x02);
    EPD_SendCommand(0x00);
    EPD_SendData(0xCF);
    EPD_SendData(0x89);
}
/**
 * @brief 清屏函数 全白
 * */
void EPD_Clear(void)
{
    uint16_t Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
    uint16_t Height = EPD_HEIGHT;
    uint16_t size = Width * Height;
    uint8_t *buf = malloc(size * sizeof(uint8_t));
    memset(buf, 0, size);
    EPD_SendCommand(0x10);
    EPD_SendDatas(buf, size);
    EPD_SendCommand(0x13);
    EPD_SendDatas(buf, size);
    EPD_SendCommand(0x04);
    EPD_ReadBusy();
    EPD_SendCommand(0x12);
    EPD_ReadBusy();
    free(buf);
}
/**
 * @brief 写图像 黑色
 * */
void EPD_Display(uint8_t *blackimage, uint8_t *redimage)
{
    uint16_t Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
    uint16_t Height = EPD_HEIGHT;
    EPD_SendCommand(0x10);
    EPD_SendDatas(blackimage, Width * Height);
    EPD_SendCommand(0x13);
    EPD_SendDatas(redimage, Width * Height);
    EPD_SendCommand(0x04);
    EPD_ReadBusy();
    EPD_SendCommand(0x12);
    EPD_ReadBusy();
}

#endif
