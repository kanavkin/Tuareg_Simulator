#ifndef STM32_ADC_H_INCLUDED
#define STM32_ADC_H_INCLUDED

#define SAMPLE_TIME_1_5      0x00
#define SAMPLE_TIME_7_5      0x01
#define SAMPLE_TIME_13_5     0x02
#define SAMPLE_TIME_28_5     0x03
#define SAMPLE_TIME_41_5     0x04
#define SAMPLE_TIME_55_5     0x05
#define SAMPLE_TIME_71_5     0x06
#define SAMPLE_TIME_239_5    0x07



void adc_set_regular_group(ADC_TypeDef * adc, uint32_t entry, uint32_t channel);
void adc_set_regular_group_length(ADC_TypeDef * adc, uint32_t length);
void adc_set_injected_sequence(ADC_TypeDef * adc, uint32_t entry, uint32_t channel);
void adc_set_injected_sequence_length(ADC_TypeDef * adc, uint32_t length);
void adc_set_sample_time(ADC_TypeDef * adc, uint32_t channel, uint32_t timing);
void adc_start_regular_group(ADC_TypeDef * adc);
void adc_start_injected_group(ADC_TypeDef * adc);




#endif // STM32_ADC_H_INCLUDED
