#include "stm32f10x.h"
#include "stm32_adc.h"



void adc_set_regular_group(ADC_TypeDef * adc, uint32_t entry, uint32_t channel)
{
    if( (channel > 17) || (entry < 1) )
    {
        return;
    }

    if(entry < 7)
    {
        adc->SQR3 &= (uint32_t) ~(0x1F << ((entry -1) * 5));
        adc->SQR3 |= (uint32_t) (channel << ((entry -1) * 5));
    }
    else if(entry < 13)
    {
        adc->SQR2 &= (uint32_t) ~(0x1F << ((entry -7) * 5));
        adc->SQR2 |= (uint32_t) (channel << ((entry -7) * 5));
    }
    else if(entry < 17)
    {
        adc->SQR1 &= (uint32_t) ~(0x1F << ((entry -13) * 5));
        adc->SQR1 |= (uint32_t) (channel << ((entry -13) * 5));
    }

}

void adc_set_regular_group_length(ADC_TypeDef * adc, uint32_t length)
{
    if((length < 1) || (length > 16))
    {
        return;
    }

    adc->SQR1 &= (uint32_t) ~(0x0F << 20);
    adc->SQR1 |= (uint32_t) ((length -1) << 20);
}






void adc_set_sample_time(ADC_TypeDef * adc, uint32_t channel, uint32_t timing)
{
    if(channel > 17)
    {
        return;
    }

    if(channel < 10)
    {
        adc->SMPR2 &= (uint32_t) ~(0x03 << (channel * 3));
        adc->SMPR2 |= (uint32_t) (timing << (channel * 3));
    }
    else if(channel < 18)
    {
        adc->SMPR1 &= (uint32_t) ~(0x03 << ((channel -10) * 3));
        adc->SMPR1 |= (uint32_t) (timing << ((channel -10) * 3));
    }

}


void adc_start_regular_group(ADC_TypeDef * adc)
{
    adc->CR2 |= ADC_CR2_SWSTART;
}


void adc_start_injected_group(ADC_TypeDef * adc)
{
    adc->CR2 |= ADC_CR2_JSWSTART;
}
