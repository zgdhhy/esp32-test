#include "myadc.h"

static const char* TAG = "ADC";
/**
 * @brief 初始化ADC
 * @param adc1_chars[out] 指向用于存储ADC特征的空结构的指针
*/
void ADC_Init(esp_adc_cal_characteristics_t *adc1_chars)
{
    if(adc1_chars != NULL)
    {
        esp_err_t ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
        if (ret == ESP_ERR_NOT_SUPPORTED)
            ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
        else if (ret == ESP_ERR_INVALID_VERSION)
            ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
        else if (ret == ESP_OK) 
            esp_adc_cal_characterize(ADC_UNIT, ADC_ATTEN, ADC_WIDTH_BIT, DEFAULT_VREF, adc1_chars);
        else ESP_LOGE(TAG, "Invalid arg");
    }
#ifdef ADC_USE_UNIT1
    adc1_config_width(ADC_WIDTH_BIT);
#endif
}
/**
 * @brief 在ADC1上设置特定通道的衰减
 * @param channel 要配置的ADC1通道
*/
void ADC1_SetChannelAtten(adc1_channel_t channel)
{
    adc1_config_channel_atten(channel, ADC_WIDTH_BIT);
}
/**
 * @brief 单次采样
 * @param channel 要采样ADC1通道
 * @return 单次采样的ADC值
*/
int ADC1_GetDataOnce(adc1_channel_t channel)
{
    return adc1_get_raw(channel);
}
/**
 * @brief 多次采样 [SAMPLING_NUMS]采样次数宏定义
 * @param channel 要采样ADC1通道
 * @return 多次采样的ADC值的平均值
*/
int ADC1_GetDataMulti(adc1_channel_t channel)
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < SAMPLING_NUMS; i++) 
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
    adc_reading /= SAMPLING_NUMS;
    return adc_reading;
}
/**
 * @brief 将采样的ADC数字值转换为电压模拟值
 * @param adc_reading  ADC数字值
 * @param adc1_chars 指向包含ADC特征的初始化结构的指针
 * @return 电压模拟值(mv)
*/
uint32_t ADC_GetVoltage(uint32_t adc_reading, const esp_adc_cal_characteristics_t *adc1_chars)
{
    return esp_adc_cal_raw_to_voltage(adc_reading, adc1_chars);
}

