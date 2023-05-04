#include "EDriver.h"
spi_device_handle_t epd_spiHandle;

/**
 * @brief 对GPIO引脚初始化
 * */
void EPD_GPIO_Init()
{
    //配置CS引脚输出 开始为高电平
    gpio_reset_pin(EPD_CS_PIN);
    gpio_set_direction(EPD_CS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(EPD_CS_PIN, 1);
    //配置DC引脚输出 开始为低电平
    gpio_reset_pin(EPD_DC_PIN);
    gpio_set_direction(EPD_DC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(EPD_DC_PIN, 0);
    //配置RST引脚输出 开始为高电平
    gpio_reset_pin(EPD_RST_PIN);
    gpio_set_direction(EPD_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(EPD_RST_PIN, 1);
    //配置BUSY引脚输入
    gpio_reset_pin(EPD_BUSY_PIN);
    gpio_set_direction(EPD_BUSY_PIN, GPIO_MODE_INPUT);
}
/**
 * @brief 对SPI初始化
 * */
void EPD_SPI_Init()
{
    spi_bus_config_t buscfg = {
            .mosi_io_num = EPD_MOSI_PIN,
            .miso_io_num = -1,
            .sclk_io_num = EPD_SCK_PIN,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = (EPD_WIDTH / 8)  * EPD_HEIGHT,
            .flags = 0
    };
    spi_bus_initialize(HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
    devcfg.queue_size = 7;
    devcfg.mode = 0;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;
    devcfg.spics_io_num = -1;
    spi_bus_add_device(HOST_ID, &devcfg, &epd_spiHandle);
}
/**
 * @brief 向EDP写一堆数据
 * */
void EPD_write_bytes(const uint8_t *data,size_t length)
{
    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    if(length > 0)
    {
        memset(&transaction,0,sizeof(spi_transaction_t));
        transaction.length = length * 8;
        transaction.tx_buffer = data;
        spi_device_transmit(epd_spiHandle,&transaction);
    }
}
/**
 * @brief 向EDP写数据
 * */
void EPD_write_byte(const uint8_t data)
{
    EPD_write_bytes(&data,1);
}
///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 复位EPD
 * */
void EPD_Reset(void)
{
    gpio_set_level(EPD_RST_PIN, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    gpio_set_level(EPD_RST_PIN, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gpio_set_level(EPD_RST_PIN, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);
}
/**
 * @brief 写命令
 * */
void EPD_SendCommand(uint8_t comd)
{
    gpio_set_level(EPD_DC_PIN, 0);
    gpio_set_level(EPD_CS_PIN, 0);
    EPD_write_byte(comd);
    gpio_set_level(EPD_CS_PIN, 1);
}
/**
 * @brief 写数据
 * */
void EPD_SendData(uint8_t data)
{
    gpio_set_level(EPD_DC_PIN, 1);
    gpio_set_level(EPD_CS_PIN, 0);
    EPD_write_byte(data);
    gpio_set_level(EPD_CS_PIN, 1);
}
/**
 * @brief 写数据
 * */
void EPD_SendDatas(const uint8_t *data,size_t length)
{
    gpio_set_level(EPD_DC_PIN, 1);
    gpio_set_level(EPD_CS_PIN, 0);
    EPD_write_bytes(data,length);
    gpio_set_level(EPD_CS_PIN, 1);
}
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
 * @brief 清屏函数 全黑
 * */
void EPD_Clear(void)
{
    uint16_t Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
    uint16_t Height = EPD_HEIGHT;
    uint16_t size = Width * Height;
    uint8_t *buf = malloc(size * sizeof(uint8_t));
    for (uint16_t i = 0; i < size; i++) buf[i] = 0xFF;
    EPD_SendCommand(0x13);
    EPD_SendDatas(buf,size);
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
    EPD_SendDatas(blackimage,Width*Height);
    EPD_SendCommand(0x12);
    EPD_ReadBusy();
}
/**
 * @brief 写图像 黑色
 * */
void EPD_Partial_Display(uint16_t x, uint16_t y,const uint8_t *image,uint16_t PART_COLUMN, uint16_t PART_PAGE)
{
    if(NULL == image) return;
    uint16_t x_start, x_end, y_start1, y_end1, y_start2, y_end2;
    x_start = x;
    x_end = x+PART_COLUMN-1;
    if(x_end > EPD_WIDTH-1) return;
    y_start1 = y / 256;
    y_start2 = y % 256;
    y_end1 = y+PART_PAGE-1;
    if(y_end1 > EPD_HEIGHT-1) return;
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
    EPD_SendDatas(image,PART_COLUMN*PART_PAGE);
    EPD_SendCommand(0x12);
    EPD_SendCommand(0x92);
    EPD_ReadBusy();
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
