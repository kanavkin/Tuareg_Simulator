/**
This is a simplified ignition system for
Yamaha XTZ 660 on STM32 platform

Note: Denso TNDF13 has inverted ignition transistor output logic
 */

/**
the startup code is adjusted for a 8 MHz external crystal

porting to other STM32:
adjust #define for STMF10X_xx HD/MD/LD
adjust linker script (compiler options)

*/



#include "stm32_libs/stm32f10x/stm32f10x.h"
#include "stm32_libs/stm32f10x/boctok/stm32f10x_gpio_boctok.h"
#include "stm32_libs/stm32f10x/boctok/stm32f10x_adc_boctok.h"

#include "types.h"
#include "crank_simulator.h"
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
#include "waveform_buffer.h"


volatile Tuareg_simulator_t Tuareg_simulator;


VU32 Crank_rpm= 1350;

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
    init_debug_pins();

    //use 16 preemption priority levels
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    UART1_Init();
    UART3_Init();

    UART_Send(DEBUG_PORT, "\r \n \r \n . \r \n . \r \n . \r \n \r \n *** This is Tuareg-Simulator *** \r \n");
    UART_Send(DEBUG_PORT, "RC 0001");

    /*
    TODO
    UART_Send(DEBUG_PORT, "\r \n config: \r \n");
    UART_Send(DEBUG_PORT, "XTZ 660 digital crank signal on GPIOD-0 \r \n");
    UART_Send(DEBUG_PORT, "-GPIOD-0 is A1 on Nucleo-F103RB- \r \n");
    UART_Send(DEBUG_PORT, "\r \n XTZ 660 ignition coil signal on GPIOB-0 \r \n");
    UART_Send(DEBUG_PORT, "-GPIOB-0 is A3 on Nucleo-F103RB- \r \n \r \n");
    */

    /*
    TODO
    DEBUG
    UART_Send(DEBUG_PORT, "TunerStudio interface ready \r\n");
    */


    /**
    initialize core components and register interface access pointers
    */
    Tuareg_simulator.pCrank_simulator= init_crank_simulation();
    init_lowspeed_timers();

    //init waveforms
    set_engine_type(XTZ750);

    reset_waveform_buffer(CRANK_WAVEFORM);

    waveform_add(CRANK_WAVEFORM, 50, 3);
    waveform_add(CRANK_WAVEFORM, 100, 3);
    waveform_add(CRANK_WAVEFORM, 60, 3);

    Tuareg_simulator.crank_simulator_mode= SMODE_WAVEFORM;
    start_crank_waveform_generator();
    start_crank_simulation(1350);

    /*
    Tuareg_simulator.crank_simulator_mode= SMODE_CONT;
    start_crank_simulation(1350);
    */

    while(1)
    {

        if(ls_timer & BIT_TIMER_1HZ)
        {
            ls_timer &= ~BIT_TIMER_1HZ;

            set_debug_led(TOGGLE);

        }



        /**
        handle TS communication
        */
        /*
        if( (ls_timer & BIT_TIMER_10HZ) || (UART_available() > SERIAL_BUFFER_THRESHOLD) )
        {
            ls_timer &= ~BIT_TIMER_10HZ;



            if (UART_available() > 0)
            {
               // ts_communication();
            }
        }
        */
    }

    return 0;
}





/******************************************************************************************************************************
sw generated irq when crank simulator has completed a crank cycle (360°)
-> new crank timing required
 ******************************************************************************************************************************/
void EXTI2_IRQHandler(void)
{
    VU32 rpmBuffer;

    //clear pending register
    EXTI->PR= EXTI_Line2;

    /**
    choose a new rpm according to simulator mode
    */
    switch(Tuareg_simulator.crank_simulator_mode)
    {
        case SMODE_STOP:

            stop_crank_simulation();
            break;

        case SMODE_WAVEFORM:

            //get current rpm
            rpmBuffer= get_simulator_rpm();

            //try to fetch the next waveform segment
            if(update_crank_generator(&rpmBuffer))
            {
                //empty buffer or generator shut off
                Tuareg_simulator.crank_simulator_mode= SMODE_STOP;
                stop_crank_simulation();
            }
            else
            {
                //success
                set_crank_rpm(rpmBuffer);
            }
            break;

        case SMODE_CONT:

            //keep current rpm
            recalc_timer_segments();
            break;


        default:

            //error, no such mode!
            stop_crank_simulation();
    }



    //set_debug_led(TOGGLE);
}

/******************************************************************************************************************************
sw generated irq
 ******************************************************************************************************************************/
void EXTI3_IRQHandler(void)
{
    //clear pending register
    EXTI->PR= EXTI_Line3;




}




