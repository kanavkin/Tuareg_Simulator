#include <math.h>

#include "stm32_libs/stm32f10x/stm32f10x.h"
#include "stm32_libs/stm32f10x/boctok/stm32f10x_gpio_boctok.h"
#include "stm32_libs/stm32f10x/boctok/stm32f10x_adc_boctok.h"

#include "types.h"
#include "sensors.h"
#include "uart.h"
#include "conversion.h"
#include "table.h"
#include "config.h"


volatile sensor_interface_t Sensors;

/**
where DMA will drop ADC data from regular group
*/
VU16 ADCBuffer[REGULAR_GROUP_LENGTH];


/**
see sensors.h for sensor layout!
*/


/**
bring up analog and digital sensors
*/
volatile sensor_interface_t * init_sensors()
{
    DMA_InitTypeDef DMA_InitStructure;

    //set ADC prescaler to 6 (12 MHz)
    RCC->CFGR |= RCC_CFGR_ADCPRE_1;
    RCC->CFGR &= ~ RCC_CFGR_ADCPRE_0;

    //clock
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    RCC->AHBENR  |= RCC_AHBENR_DMA1EN;

    //GPIO ADC CH0..7
    GPIO_configure(GPIOA, 0, GPIO_ANALOG);
    GPIO_configure(GPIOA, 1, GPIO_ANALOG);
    GPIO_configure(GPIOA, 2, GPIO_ANALOG);
    GPIO_configure(GPIOA, 3, GPIO_ANALOG);
    GPIO_configure(GPIOA, 4, GPIO_ANALOG);
    GPIO_configure(GPIOA, 5, GPIO_ANALOG);
    GPIO_configure(GPIOA, 6, GPIO_ANALOG);
    GPIO_configure(GPIOA, 7, GPIO_ANALOG);

    //GPIO run + crash + spare 1/2 + debug
    GPIO_configure(GPIOB, 4, GPIO_IN_PUD);
    GPIO_configure(GPIOC, 0, GPIO_IN_PUD);
    GPIO_configure(GPIOC, 2, GPIO_IN_PUD);
    GPIO_configure(GPIOC, 3, GPIO_IN_PUD);
    GPIO_configure(GPIOC, 5, GPIO_IN_PUD);

    /**
    ADC setup low fat variant:
    CR1 ->  use SCAN mode and IRQ on injected conversion end
    CR2 ->  sw external trigger for regular + injected groups,
            use DMA for regular group, data align right, independent mode,
            no continuous conversion
    */
    ADC1->CR1= ADC_CR1_SCAN | ADC_CR1_JEOCIE;
    ADC1->CR2= ADC_CR2_EXTTRIG | ADC_CR2_JEXTTRIG | ADC_CR2_EXTSEL | ADC_CR2_JEXTSEL | ADC_CR2_DMA | ADC_CR2_ADON;

    //perform ADC calibration
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while(ADC1->CR2 & ADC_CR2_RSTCAL);
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL);

    /**
    ADC channels setup

    Regular group handling:
    The enumerator value of ASENSOR_  in sensors_t is used as its address in ADCBuffer[].
    The ADC converts the regular group channels in the order we set through adc_set_regular_group.
    DMA writes the conversion results to ADCBuffer[]
    -> The channel order must match the enumerator values!
    -> keep REGULAR_GROUP_LENGTH up to date!

    sensor:     O2   CLT    SPARE
                |     |       |
    group nr.   1 2 3 4 5 6 7 8
                | | | | | | | |
    ADCBuffer[] 0 1 2 3 4 5 6 7

    Injected group handling:
    MAP sensor is in the injected group
    ADC conversion result is read in irq

    ADC clock is 12MHz, conversion time will be 12.5 + sample_time * T_adc
    so total regular conversion time should be ~9us
    */
    adc_set_sample_time(ADC1, ASENSOR_O2_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_TPS_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_MAP_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_IAT_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_CLT_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_VBAT_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_KNOCK_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_BARO_CH, SAMPLE_TIME_7_5);
    adc_set_sample_time(ADC1, ASENSOR_SPARE_CH, SAMPLE_TIME_7_5);

    adc_set_regular_group(ADC1, 1, ASENSOR_O2_CH);
    adc_set_regular_group(ADC1, 2, ASENSOR_TPS_CH);
    adc_set_regular_group(ADC1, 3, ASENSOR_IAT_CH);
    adc_set_regular_group(ADC1, 4, ASENSOR_CLT_CH);
    adc_set_regular_group(ADC1, 5, ASENSOR_VBAT_CH);
    adc_set_regular_group(ADC1, 6, ASENSOR_KNOCK_CH);
    adc_set_regular_group(ADC1, 7, ASENSOR_BARO_CH);
    adc_set_regular_group(ADC1, 8, ASENSOR_SPARE_CH);

    //set the number of channels in the regular group in sensors.h!
    adc_set_regular_group_length(ADC1, REGULAR_GROUP_LENGTH);


    /**
    setting up the injected group is a bit tricky:
    when we have only one conversion in group, ADC_JSQR.JL is 0 and only JSQ4 entry will
    define which channel to convert.
    Conversion result will be in ADC1->JDR1 !
    */
    ADC1->JSQR= (U32) (ASENSOR_MAP_CH << 15);


    //DMA for ADC
    DMA_InitStructure.DMA_BufferSize= REGULAR_GROUP_LENGTH;
    DMA_InitStructure.DMA_Priority= DMA_Priority_High;
    DMA_InitStructure.DMA_M2M= DMA_M2M_Disable;
    DMA_InitStructure.DMA_DIR= DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc= DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc= DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode= DMA_Mode_Circular;
    DMA_InitStructure.DMA_PeripheralDataSize= DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize= DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_PeripheralBaseAddr= (U32)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr= (U32)ADCBuffer;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    //enable DMA1_Channel1 and its irq
    DMA1_Channel1->CCR |= DMA_CCR1_EN;
    DMA1_Channel1->CCR |= DMA_IT_TC;


    /**
    NVIC
    */

    //DMA1 channel (prio 7)
    NVIC_SetPriority(DMA1_Channel1_IRQn, 7UL);
    NVIC_ClearPendingIRQ(DMA1_Channel1_IRQn);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    //ADC1-2 irq (prio 6)
    NVIC_SetPriority(ADC1_2_IRQn, 6UL);
    NVIC_ClearPendingIRQ(ADC1_2_IRQn);
    NVIC_EnableIRQ(ADC1_2_IRQn);

    return &Sensors;
}



/**
handling digital sensors seems so easy
compared to analog ones ;)
*/
void read_digital_sensors()
{
    //DSENSOR_SPARE2
    if(GPIOB->IDR & GPIO_IDR_IDR4)
    {
        Sensors.digital_sensors |= DSENSOR_SPARE2;
    }
    else
    {
        Sensors.digital_sensors &= ~DSENSOR_SPARE2;
    }

    //DSENSOR_SPARE1
    if(GPIOC->IDR & GPIO_IDR_IDR0)
    {
        Sensors.digital_sensors |= DSENSOR_SPARE1;
    }
    else
    {
        Sensors.digital_sensors &= ~DSENSOR_SPARE1;
    }

    //DSENSOR_RUN
    if(GPIOC->IDR & GPIO_IDR_IDR2)
    {
        Sensors.digital_sensors |= DSENSOR_RUN;
    }
    else
    {
        Sensors.digital_sensors &= ~DSENSOR_RUN;
    }

    //DSENSOR_CRASH
    if(GPIOC->IDR & GPIO_IDR_IDR3)
    {
        Sensors.digital_sensors |= DSENSOR_CRASH;
    }
    else
    {
        Sensors.digital_sensors &= ~DSENSOR_CRASH;
    }

    //DSENSOR_DEBUG
    if(GPIOC->IDR & GPIO_IDR_IDR5)
    {
        Sensors.digital_sensors |= DSENSOR_DEBUG;
    }
    else
    {
        Sensors.digital_sensors &= ~DSENSOR_DEBUG;
    }

}


/**
MAP sensor readout as injected conversion
(can be triggered synchronous to crank movement)
*/
void ADC1_2_IRQHandler()
{

}


/**
The regular group conversion is triggered by lowspeed_timer
every 20 ms (50Hz)
*/
void DMA1_Channel1_IRQHandler()
{

}
