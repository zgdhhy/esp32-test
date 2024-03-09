#include "EpaperHal.h"
#include "Edriver.h"

spi_device_handle_t epd_spiHandle;
/**
 * @
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
            .max_transfer_sz = (EPD_WIDTH / 8 + 1) * EPD_HEIGHT,
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
void EPD_write_bytes(const uint8_t *data, size_t length)
{
    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    if (length > 0)
    {
        memset(&transaction, 0, sizeof(spi_transaction_t));
        transaction.length = length * 8;
        transaction.tx_buffer = data;
        spi_device_transmit(epd_spiHandle, &transaction);
    }
}
/**
 * @brief 向EDP写数据
 * */
void EPD_write_byte(const uint8_t data)
{
    EPD_write_bytes(&data, 1);
}


/**
 * @brief 复位EPD
 * */
void EPD_Reset(void)
{
    gpio_set_level(EPD_RST_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(EPD_RST_PIN, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(EPD_RST_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
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
 * @brief 写一个数据
 * */
void EPD_SendData(uint8_t data)
{
    gpio_set_level(EPD_DC_PIN, 1);
    gpio_set_level(EPD_CS_PIN, 0);
    EPD_write_byte(data);
    gpio_set_level(EPD_CS_PIN, 1);
}
/**
 * @brief 写多个数据
 * */
void EPD_SendDatas(const uint8_t *data, size_t length)
{
    gpio_set_level(EPD_DC_PIN, 1);
    gpio_set_level(EPD_CS_PIN, 0);
    EPD_write_bytes(data, length);
    gpio_set_level(EPD_CS_PIN, 1);
}