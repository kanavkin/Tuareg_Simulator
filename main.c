/**
This is a simplified ignition system for
Yamaha XTZ 660 on STM32 platform

Note: Denso TNDF13 has inverted ignition transistor output logic
 */

/**
a little performance analysis:

    This was with ATmega328p @ 16 MHz:
        -calc_rpm() takes about 39 us
        -calc_ignition_timings() takes about 103,5 us
        -we are entering this section about 17,6 us after the corresponding
            trigger signal edge
        -all the ignition calculation is finished after about 160 us
            after the corresponding trigger signal edge of IGNITION_RECALC_POSITION
            this is about 8 deg @ 8500 rpm
        -the ignition timer is set up after about 22,2 us after POSITION_A2
            trigger signal edge, this is about 1,13 deq @ 8500 rpm (0.7 deg at average rpms)

    In STM32F103 @ 72 MHz:
        -ignition set up time (delay from pickup signal trigger to transistor signal change) is about 4us
        -accuracy: measured 37,6° advance @ 8500 rpm when 38° was set up (7,4us off)
*/


#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"
#include "stm32_libs/boctok/stm32_adc.h"

#include "types.h"
#include "decoder.h"
#include "ignition.h"
#include "scheduler.h"
#include "uart.h"
#include "conversion.h"
#include "lowspeed_timers.h"
#include "TunerStudio.h"
#include "config.h"
#include "table.h"
#include "eeprom.h"
#include "sensors.h"

#include "debug.h"
#include "Tuareg.h"

#define DIAG_MSG_POSITION POSITION_D2
#define CYLINDER_SENSOR_POSITION POSITION_D2

/**
global status object
*/
volatile Tuareg_t Tuareg;

//volatile decoder_t * main_decoder = NULL;
//volatile ignition_timing_t ignition_timing;
//volatile sensor_interface_t * hw_sensors = NULL;

/**
Tuareg IRQ priorities:

1    decoder:
    crank pickup (EXTI) -> EXTI0_IRQn
    crank pickup filter (timer 2) -> TIM2_IRQn

2   scheduler (timer 3) -> TIM3_IRQn

3   cis (part of decoder) (EXTI) -> EXTI1_IRQn

4   decoder (sw EXTI) -> EXTI2_IRQn

5   lowspeed timer (systick) -> SysTick_IRQn

6   ADC injected conversion complete -> ADC1_2_IRQn

7   ADC conversion complete (DMA transfer) -> DMA1_Channel1_IRQn

10  ignition timing recalculation (sw EXTI) -> EXTI3_IRQn

14  tunerstudio (usart 1) -> USART1_IRQn

15  debug com  (usart 3) -> USART3_IRQn

*/

/**
Tuareg EXTI ressources:

EXTI0:  PORTB0  --> crank pickup signal

EXTI1:

EXTI2:  -sw-    --> decoder int

EXTI3:  -sw-    --> ignition irq

EXTI4: ?

*/


/**
how to use platform ressources:



*/




int main(void)
{
    U32 config_load_status;

    //starting primary initialisation
    Tuareg.Runmode= TMODE_PRIMING;

    //use 16 preemption priority levels
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    UART1_Init();
    UART3_Init();

    UART_Send(DEBUG_PORT, "\r \n \r \n . \r \n . \r \n . \r \n \r \n *** This is Tuareg, lord of the Sahara *** \r \n");
    UART_Send(DEBUG_PORT, "RC 0001");
    UART_Send(DEBUG_PORT, "\r \n config: \r \n");
    UART_Send(DEBUG_PORT, "XTZ 660 digital crank signal on GPIOD-0 \r \n");
    UART_Send(DEBUG_PORT, "-GPIOD-0 is A1 on Nucleo-F103RB- \r \n");
    UART_Send(DEBUG_PORT, "\r \n XTZ 660 ignition coil signal on GPIOB-0 \r \n");
    UART_Send(DEBUG_PORT, "-GPIOB-0 is A3 on Nucleo-F103RB- \r \n \r \n");

    //DEBUG
    UART_Send(DEBUG_PORT, "TunerStudio interface ready \r\n");

    /**
    set up eeprom for loading configuration data
    */
    init_eeprom();

    //actually sets table dimension, could be removed
    init_3Dtables();

    //begin config load
    Tuareg.Runmode= TMODE_CONFIGLOAD;

    //loading the config data is important to us, failure in loading forces "limp home mode"
    config_load_status= load_ConfigData();

    if(config_load_status != 0)
    {
        //save to error register
        Tuareg.Errors |= TERROR_CONFIG;

        //as we can hardly save some error logs in this system condition, print some debug messages only
        UART_Send(DEBUG_PORT, "\r \n *** FAILED to load config data !");
    }

    //set 2D table dimension and link table data to config pages
    init_2Dtables();

    //ready to initialise further system hardware
    Tuareg.Runmode= TMODE_HWINIT;

    /**
    initialize core components and register interface access pointers
    */
    Tuareg.sensor_interface= init_sensors();
    Tuareg.decoder= init_decoder();
    //Tuareg.ignition_timing= &ignition_timing;
    init_ignition(&Tuareg.ignition_timing);
    init_scheduler();
    init_lowspeed_timers();

    //ready to carry out system migration
    Tuareg.Runmode= TMODE_MIGRATION;

    /**
    Check if any data items need updating
    (Occurs with firmware updates)
    */
    migrate_configData();


    //DEBUG
    init_debug_pins();

    //serial monitor
    #ifdef SERIAL_MONITOR
    UART1_Send("\r \n serial monitor loaded!");
    #endif

    /*
    initialize fuel pump to maintain normal fuel pressure
    */


    /**
    system initialisation finished
    now decide which system run mode
    follows
    */
    if(Tuareg.Errors & TERROR_CONFIG)
    {
        Tuareg.Runmode= TMODE_LIMP;
    }
    else
    {
        Tuareg.Runmode= TMODE_RUN;
    }


    while(1)
    {
        /**
        handle TS communication
        */
        if( (ls_timer & BIT_TIMER_10HZ) || (UART_available() > SERIAL_BUFFER_THRESHOLD) )
        {
            ls_timer &= ~BIT_TIMER_10HZ;

            //serial monitor
            #ifdef SERIAL_MONITOR
            while( monitor_available() )
            {
                monitor_print();
            }
            #endif // SERIAL_MONITOR

            if (UART_available() > 0)
            {
                ts_communication();
            }
        }

    }

    return 0;
}





/******************************************************************************************************************************
sw generated irq when decoder has
updated crank_position based on crank pickup signal
or decoder timeout occurred!
 ******************************************************************************************************************************/
void EXTI2_IRQHandler(void)
{
    //clear pending register
    EXTI->PR= EXTI_Line2;

    /**
    check if this is a decoder timeout (engine has stalled)
    -> shut down coils, injectors and fuel pump
    or
    a regular crank position update event
    -> trigger coil handling
       (immediate action: 4us behind pickup signal edge!)
    or
    decoder has lost sync
    */
    if((Tuareg.decoder->engine_rpm == 0) && (Tuareg.decoder->crank_position == UNDEFINED_POSITION))
    {
        //decoder timeout


            //DEBUG
            UART_Send(DEBUG_PORT, "\r \n decoder timeout");


    }
    else if(Tuareg.decoder->crank_position == Tuareg.ignition_timing.coil_on_pos)
    {
        trigger_coil_by_timer(Tuareg.ignition_timing.coil_on_timing, ON);
    }
    else if(Tuareg.decoder->crank_position == Tuareg.ignition_timing.coil_off_pos)
    {
        trigger_coil_by_timer(Tuareg.ignition_timing.coil_off_timing, OFF);
    }

    /**
    read the MAP sensor
    */
    adc_start_injected_group(SENSOR_ADC);

    /**
    cylinder identification sensor handling inside decoder module!
    */

}

/******************************************************************************************************************************
sw generated irq when spark has fired
-> recalculate ignition timing
 ******************************************************************************************************************************/
void EXTI3_IRQHandler(void)
{
    //clear pending register
    EXTI->PR= EXTI_Line3;

    /**
    recalculate ignition timing
    */
    calc_ignition_timings(&Tuareg.ignition_timing);


    //DEBUG
    set_debug_led(TOGGLE);
}




