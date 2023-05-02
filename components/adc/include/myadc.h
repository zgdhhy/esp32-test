#ifndef MYADC_H
#define MYADC_H
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

//检查ADC校准值是否被烧入eFuse时的校准值类型
#if CONFIG_IDF_TARGET_ESP32        
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_VREF
#elif CONFIG_IDF_TARGET_ESP32S2
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32C3
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP_FIT
#endif

#define ADC_USE_UNIT1
#define ADC_UNIT        ADC_UNIT_1          // 使用的ADC单元
#define ADC_ATTEN       ADC_ATTEN_DB_11     // 使用的ADC衰减参数
#define ADC_WIDTH_BIT   ADC_WIDTH_BIT_12    // 使用的ADC采集分辨率
#define DEFAULT_VREF    1100                // 默认ADC参考电压mV(仅在ESP32中，当eFuse值不可用时使用)
#define SAMPLING_NUMS   16                  // 重复采样迭代次数

void ADC_Init(esp_adc_cal_characteristics_t *adc1_chars);
void ADC1_SetChannelAtten(adc1_channel_t channel);
int ADC1_GetDataOnce(adc1_channel_t channel);
int ADC1_GetDataMulti(adc1_channel_t channel);
uint32_t ADC_GetVoltage(uint32_t adc_reading, const esp_adc_cal_characteristics_t *adc1_chars);

#endif