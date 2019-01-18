#include <math.h>

#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"
#include "stm32_libs/boctok/stm32_adc.h"

#include "types.h"
#include "sensors.h"
#include "uart.h"
#include "conversion.h"
#include "table.h"
#include "config.h"
#include "decoder.h"

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
    U32 map_calc;

    //MAP sensor handled by injected group
    if(ADC1->SR & ADC_SR_JEOC)
    {
        //clear JEOC by write 0
        ADC1->SR &= ~(U32) ADC_SR_JEOC;

        /**
        MAP sensor shall be read synchronous to crank rotation,
        (measuring across the entire engine cycle of 720 deg)
        its averages value according to ASENSOR_MAP_AVG_THRES ->
        T_upd= ASENSOR_MAP_AVG_THRES * 1ms

        TODO:
        handle sync lost events from decoder (reset integrator)
        */
        if( (ADC1->JDR1 >= ASENSOR_MAP_MIN_THRES) && (ADC1->JDR1 <= ASENSOR_MAP_MAX_THRES) )
        {
            /**
            valid reading
            */

            //store to average buffer
            Sensors.map_integrator += ADC1->JDR1;
            Sensors.map_integrator_count++;

            //calculate the MAP from sensor reading
            if(Sensors.map_integrator_count >= ASENSOR_MAP_AVG_THRES)
            {
                /**
                MAP sensor data conversion by inverse linear function
                x = ( #_adc * (phi*l) - (phi*n) ) / (phi*m)
                scaling factor phi is for maximum calculation precision
                phi = 4095 * beta
                constant l reflects the analogue input scaling and 12 bit adc resolution:
                l = u_ref/ (k * 4095)
                k = u_sensor / u_adc = R_in/R_ges
                map_calib_xxx is the actual calibration value multiplied by phi:
                configPage9.MAP_calib_L := (phi*l)
                ...and so on...
                */
                map_calc= (Sensors.map_integrator / Sensors.map_integrator_count) * configPage9.MAP_calib_L;

                if(configPage9.MAP_calib_M)
                {
                    /**
                    do not ever delete by zero
                    so we test if MAP_calib_M is set to something usable
                    -> calibration loading success should be monitored and
                    there should be default values if an error occurred while loading
                    ... but who can be sure for sure... ;9
                    */
                    Sensors.MAP= map_calc / configPage9.MAP_calib_M;

                    /**
                    provide intake vacuum figure if possible
                    */
                    if( (Sensors.active_sensors & ASENSOR_BARO_ACT) && (Sensors.BARO > Sensors.MAP) )
                    {
                        Sensors.Intake_Vacuum= Sensors.BARO - Sensors.MAP;
                    }
                    else
                    {
                        Sensors.Intake_Vacuum =0;
                    }
                }

                //reset average buffer
                Sensors.map_integrator =0;
                Sensors.map_integrator_count =0;

                //mark sensor as active, reset error counter
                Sensors.active_sensors |= ASENSOR_MAP_ACT;
                Sensors.error_counter[ASENSOR_MAP] =0;
            }

        }
        else
        {
            /**
            invalid reading, do error handling
            */
            if(Sensors.error_counter[ASENSOR_MAP] < ASENSOR_MAP_ERROR_THRES)
            {
                Sensors.error_counter[ASENSOR_MAP]++;
            }
            else
            {
                /**
                sensor temporarily disturbed!
                reset average buffer
                mark sensor inactive
                */
                Sensors.MAP =0;
                Sensors.Intake_Vacuum =0;
                Sensors.map_integrator =0;
                Sensors.map_integrator_count =0;
                Sensors.active_sensors &= ~ASENSOR_MAP_ACT;
            }

        }

    }

}


/**
The regular group conversion is triggered by lowspeed_timer
every 20 ms (50Hz)
*/
void DMA1_Channel1_IRQHandler()
{
    U32 lin_calc;
    S16 delta_TPS;

    //DMA1 Channel1 Transfer Complete interrupt
    if(DMA1->ISR & DMA1_IT_TC1)
    {
        //Clear all DMA1 interrupt pending bits
        DMA1->IFCR= DMA1_IT_GL1;

        Sensors.loop_count++;

        /**
        TPS and O2 sensors have to be read every loop
        */

        /**
        TPS sensor shall be read every 100ms,
        its averages value according to ASENSOR_TPS_AVG_THRES ->
        T_upd= ASENSOR_TPS_AVG_THRES * 20ms
        */
        if( (ADCBuffer[ASENSOR_TPS] >= ASENSOR_TPS_MIN_THRES) && (ADCBuffer[ASENSOR_TPS] <= ASENSOR_TPS_MAX_THRES) )
        {
            /**
            valid reading
            */

            //store to average buffer
            Sensors.average[ASENSOR_TPS] += ADCBuffer[ASENSOR_TPS];
            Sensors.average_count[ASENSOR_TPS]++;

            //calculate the TP from sensor reading
            if(Sensors.average_count[ASENSOR_TPS] >= ASENSOR_TPS_AVG_THRES)
            {
                /**
                save last value
                and
                calculate throttle opening by table interpolation
                */
                Sensors.last_TPS= Sensors.TPS;
                Sensors.TPS= table2D_getValue(&TPS_calib_table, Sensors.average[ASENSOR_TPS] / Sensors.average_count[ASENSOR_TPS]);

                /**
                throttle transient calculation
                as the change in airflow restriction past the throttle is a inverse logarithmic curve
                we use a weighted measure:
                ddt_TPS= (tps_new - tps_old) * (101 - tps_old)
                but suppress little spikes
                */
                delta_TPS= ((S16) Sensors.TPS - (S16) Sensors.last_TPS);

                if( ((delta_TPS < 0) && (-delta_TPS < DELTA_TPS_THRES)) || ((delta_TPS >= 0) && (delta_TPS < DELTA_TPS_THRES)) )
                {
                    Sensors.ddt_TPS =0;
                }
                else
                {
                    Sensors.ddt_TPS= delta_TPS * ((S16)(101 - (S16) Sensors.last_TPS));
                }

                //reset average buffer
                Sensors.average[ASENSOR_TPS] =0;
                Sensors.average_count[ASENSOR_TPS] =0;

                //mark sensor as active, reset error counter
                Sensors.active_sensors |= ASENSOR_TPS_ACT;
                Sensors.error_counter[ASENSOR_TPS] =0;
            }
        }
        else
        {
            /**
            invalid reading, do error handling
            */
            if(Sensors.error_counter[ASENSOR_TPS] < ASENSOR_TPS_ERROR_THRES)
            {
                Sensors.error_counter[ASENSOR_TPS]++;
            }
            else
            {
                /**
                sensor temporarily disturbed!
                reset average buffer
                mark sensor inactive
                */
                Sensors.TPS =0;
                Sensors.ddt_TPS =0;
                Sensors.last_TPS =0;
                Sensors.average[ASENSOR_TPS] =0;
                Sensors.average_count[ASENSOR_TPS] =0;
                Sensors.active_sensors &= ~ASENSOR_TPS_ACT;
            }

        }


        /**
        O2 sensor shall be read every 100ms,
        its averages value according to ASENSOR_O2_AVG_THRES ->
        T_upd= ASENSOR_O2_AVG_THRES * 20ms
        */
        if( (ADCBuffer[ASENSOR_O2] >= ASENSOR_O2_MIN_THRES) && (ADCBuffer[ASENSOR_O2] <= ASENSOR_O2_MAX_THRES) )
        {
            /**
            valid reading
            */

            //store to average buffer
            Sensors.average[ASENSOR_O2] += ADCBuffer[ASENSOR_O2];
            Sensors.average_count[ASENSOR_O2]++;

            //calculate the TP from sensor reading
            if(Sensors.average_count[ASENSOR_O2] >= ASENSOR_O2_AVG_THRES)
            {
                /**
                O2 sensor data conversion by inverse linear function
                see MAP sensor handling for explanation!
                */
                lin_calc= (Sensors.average[ASENSOR_O2] / Sensors.average_count[ASENSOR_O2]) * configPage9.O2_calib_L;

                if(configPage9.O2_calib_M)
                {
                    Sensors.O2= lin_calc / configPage9.O2_calib_M;
                }

                //reset average buffer
                Sensors.average[ASENSOR_O2] =0;
                Sensors.average_count[ASENSOR_O2] =0;

                //mark sensor as active, reset error counter
                Sensors.active_sensors |= ASENSOR_O2_ACT;
                Sensors.error_counter[ASENSOR_O2] =0;
            }
        }
        else
        {
            /**
            invalid reading, do error handling
            */
            if(Sensors.error_counter[ASENSOR_O2] < ASENSOR_O2_ERROR_THRES)
            {
                Sensors.error_counter[ASENSOR_O2]++;
            }
            else
            {
                /**
                sensor temporarily disturbed!
                reset average buffer
                mark sensor inactive
                */
                Sensors.O2 =0;
                Sensors.average[ASENSOR_O2] =0;
                Sensors.average_count[ASENSOR_O2] =0;
                Sensors.active_sensors &= ~ASENSOR_O2_ACT;
            }

        }



        /**
        VBAT sensor shall be updated every 500ms,
        its averages value according to ASENSOR_VBAT_AVG_THRES ->
        T_upd= ASENSOR_VBAT_AVG_THRES * 20ms * loop_count
        */
         if(Sensors.loop_count == 5)
        {
            if( (ADCBuffer[ASENSOR_VBAT] >= ASENSOR_VBAT_MIN_THRES) && (ADCBuffer[ASENSOR_VBAT] <= ASENSOR_VBAT_MAX_THRES) )
            {
                /**
                valid reading
                */

                //store to average buffer
                Sensors.average[ASENSOR_VBAT] += ADCBuffer[ASENSOR_VBAT];
                Sensors.average_count[ASENSOR_VBAT]++;

                //calculate battery voltage from sensor reading
                if(Sensors.average_count[ASENSOR_VBAT] >= ASENSOR_VBAT_AVG_THRES)
                {
                    /**
                    VBAT sensor data conversion by inverse linear function
                    see MAP sensor handling for explanation!
                    */
                    lin_calc= (Sensors.average[ASENSOR_VBAT] / Sensors.average_count[ASENSOR_VBAT]) * configPage9.VBAT_calib_L;

                    if(configPage9.VBAT_calib_M)
                    {
                        Sensors.VBAT= lin_calc / configPage9.VBAT_calib_M;
                    }

                    //reset average buffer
                    Sensors.average[ASENSOR_VBAT] =0;
                    Sensors.average_count[ASENSOR_VBAT] =0;

                    //mark sensor as active, reset error counter
                    Sensors.active_sensors |= ASENSOR_VBAT_ACT;
                    Sensors.error_counter[ASENSOR_VBAT] =0;
                }
            }
            else
            {
                /**
                invalid reading, do error handling
                */
                if(Sensors.error_counter[ASENSOR_VBAT] < ASENSOR_VBAT_ERROR_THRES)
                {
                    Sensors.error_counter[ASENSOR_VBAT]++;
                }
                else
                {
                    /**
                    sensor temporarily disturbed!
                    reset average buffer
                    mark sensor inactive
                    */
                    Sensors.VBAT =0;
                    Sensors.average[ASENSOR_VBAT] =0;
                    Sensors.average_count[ASENSOR_VBAT] =0;
                    Sensors.active_sensors &= ~ASENSOR_VBAT_ACT;
                }

            }

        }


        /**
        BARO, IAT and CLT sensors shall be updated every second,
        its averages value according to ASENSOR_x_AVG_THRES ->
        T_upd= ASENSOR_x_AVG_THRES * 20ms * loop_count
        */
        if(Sensors.loop_count == 10)
        {
            //IAT
            if( (ADCBuffer[ASENSOR_IAT] >= ASENSOR_IAT_MIN_THRES) && (ADCBuffer[ASENSOR_IAT] <= ASENSOR_IAT_MAX_THRES) )
            {
                /**
                valid reading
                */

                //store to average buffer
                Sensors.average[ASENSOR_IAT] += ADCBuffer[ASENSOR_IAT];
                Sensors.average_count[ASENSOR_IAT]++;

                if(Sensors.average_count[ASENSOR_IAT] >= ASENSOR_IAT_AVG_THRES)
                {
                    /**
                    IAT sensor data conversion by table interpolation
                    */
                    Sensors.IAT= table2D_getValue(&IAT_calib_table, Sensors.average[ASENSOR_IAT] / Sensors.average_count[ASENSOR_IAT]);

                    //reset average buffer
                    Sensors.average[ASENSOR_IAT] =0;
                    Sensors.average_count[ASENSOR_IAT] =0;

                    //mark sensor as active, reset error counter
                    Sensors.active_sensors |= ASENSOR_IAT_ACT;
                    Sensors.error_counter[ASENSOR_IAT] =0;
                }
            }
            else
            {
                /**
                invalid reading, do error handling
                */
                if(Sensors.error_counter[ASENSOR_IAT] < ASENSOR_IAT_ERROR_THRES)
                {
                    Sensors.error_counter[ASENSOR_IAT]++;
                }
                else
                {
                    /**
                    sensor temporarily disturbed!
                    reset average buffer
                    mark sensor inactive
                    */
                    Sensors.IAT =0;
                    Sensors.average[ASENSOR_IAT] =0;
                    Sensors.average_count[ASENSOR_IAT] =0;
                    Sensors.active_sensors &= ~ASENSOR_IAT_ACT;
                }

            }

            //CLT
            if( (ADCBuffer[ASENSOR_CLT] >= ASENSOR_CLT_MIN_THRES) && (ADCBuffer[ASENSOR_CLT] <= ASENSOR_CLT_MAX_THRES) )
            {
                /**
                valid reading
                */

                //store to average buffer
                Sensors.average[ASENSOR_CLT] += ADCBuffer[ASENSOR_CLT];
                Sensors.average_count[ASENSOR_CLT]++;

                //calculate the TP from sensor reading
                if(Sensors.average_count[ASENSOR_CLT] >= ASENSOR_CLT_AVG_THRES)
                {
                    /**
                    CLT sensor data conversion by table interpolation
                    */
                    Sensors.CLT= table2D_getValue(&CLT_calib_table, Sensors.average[ASENSOR_CLT] / Sensors.average_count[ASENSOR_CLT]);

                    //reset average buffer
                    Sensors.average[ASENSOR_CLT] =0;
                    Sensors.average_count[ASENSOR_CLT] =0;

                    //mark sensor as active, reset error counter
                    Sensors.active_sensors |= ASENSOR_CLT_ACT;
                    Sensors.error_counter[ASENSOR_CLT] =0;
                }
            }
            else
            {
                /**
                invalid reading, do error handling
                */
                if(Sensors.error_counter[ASENSOR_CLT] < ASENSOR_CLT_ERROR_THRES)
                {
                    Sensors.error_counter[ASENSOR_CLT]++;
                }
                else
                {
                    /**
                    sensor temporarily disturbed!
                    reset average buffer
                    mark sensor inactive
                    */
                    Sensors.CLT =0;
                    Sensors.average[ASENSOR_CLT] =0;
                    Sensors.average_count[ASENSOR_CLT] =0;
                    Sensors.active_sensors &= ~ASENSOR_CLT_ACT;
                }

            }

            //BARO
            if( (ADCBuffer[ASENSOR_BARO] >= ASENSOR_BARO_MIN_THRES) && (ADCBuffer[ASENSOR_BARO] <= ASENSOR_BARO_MAX_THRES) )
            {
                /**
                valid reading
                */

                //store to average buffer
                Sensors.average[ASENSOR_BARO] += ADCBuffer[ASENSOR_BARO];
                Sensors.average_count[ASENSOR_BARO]++;

                if(Sensors.average_count[ASENSOR_BARO] >= ASENSOR_BARO_AVG_THRES)
                {
                    /**
                    BARO sensor data conversion by inverse linear function
                    see MAP sensor handling for explanation!
                    */
                    lin_calc= (Sensors.average[ASENSOR_BARO] / Sensors.average_count[ASENSOR_BARO]) * configPage9.BARO_calib_L;

                    if(configPage9.BARO_calib_M)
                    {
                        Sensors.BARO= lin_calc / configPage9.BARO_calib_M;
                    }

                    //reset average buffer
                    Sensors.average[ASENSOR_BARO] =0;
                    Sensors.average_count[ASENSOR_BARO] =0;

                    //mark sensor as active, reset error counter
                    Sensors.active_sensors |= ASENSOR_BARO_ACT;
                    Sensors.error_counter[ASENSOR_BARO] =0;
                }
            }
            else
            {
                /**
                invalid reading, do error handling
                */
                if(Sensors.error_counter[ASENSOR_BARO] < ASENSOR_BARO_ERROR_THRES)
                {
                    Sensors.error_counter[ASENSOR_BARO]++;
                }
                else
                {
                    /**
                    sensor temporarily disturbed!
                    reset average buffer
                    mark sensor inactive
                    */
                    Sensors.BARO =0;
                    Sensors.Intake_Vacuum =0;
                    Sensors.average[ASENSOR_BARO] =0;
                    Sensors.average_count[ASENSOR_BARO] =0;
                    Sensors.active_sensors &= ~ASENSOR_BARO_ACT;
                }

            }


            /**
            all sensors read - reset loop count
            */
            Sensors.loop_count =0;

        }

    }
}
