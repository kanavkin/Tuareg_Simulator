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
#include "comm.h"
#include "config.h"
#include "table.h"
#include "eeprom.h"
#include "sensors.h"

#include "debug.h"
#include "Tuareg.h"
#include "waveform_buffer.h"


volatile Tuareg_simulator_t Tuareg_simulator;



/**
bluepill config:
RAM 20k
FLASH 64k



*/



int main(void)
{
    init_debug_pins();

    //use 16 preemption priority levels
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    UART1_Init();

    UART_Send(TS_PORT, "\r\n\r\n.\r\n.\r\n.\r\n\r\n*** This is Tuareg-Simulator *** \r\n");
    UART_Send(TS_PORT, "\r\nRC 0001");
    UART_Send(TS_PORT, "\r\nconfig:");
    UART_Send(TS_PORT, "\r\ncrank signal on PORTB 10\r\n\r\n");

    /**
    initialize core components and register interface access pointers
    */
    Tuareg_simulator.pCrank_simulator= init_crank_simulator();
    init_lowspeed_timers();

    //init waveforms
    set_engine_type(XTZ750);

    /*
    reset_waveform_buffer(CRANK_WAVEFORM);

    waveform_add(CRANK_WAVEFORM, 50, 3);
    waveform_add(CRANK_WAVEFORM, 100, 3);
    waveform_add(CRANK_WAVEFORM, 60, 3);

    Tuareg_simulator.crank_simulator_mode= SMODE_WAVEFORM;
    start_crank_waveform_generator();

    start_crank_simulation(1350);
    */

    //default action
    set_sweep_mode();

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
        if( (ls_timer & BIT_TIMER_10HZ) || (UART_available() > SERIAL_BUFFER_THRESHOLD) )
        {
            ls_timer &= ~BIT_TIMER_10HZ;


            comm_periodic();

        }

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
            rpmBuffer= Tuareg_simulator.pCrank_simulator->rpm;

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

            set_crank_rpm(Tuareg_simulator.pCrank_simulator->cont_rpm);

            //keep current rpm
            calc_timer_segments();
            break;

        case SMODE_SWEEP:

            if(Tuareg_simulator.pCrank_simulator->sweep_counter >= Tuareg_simulator.pCrank_simulator->sweep_hold )
            {
                Tuareg_simulator.pCrank_simulator->sweep_counter =0;

                rpmBuffer= Tuareg_simulator.pCrank_simulator->rpm + Tuareg_simulator.pCrank_simulator->sweep_increment;

                if(rpmBuffer <= Tuareg_simulator.pCrank_simulator->sweep_end)
                {
                    //increase rpm
                    set_crank_rpm(rpmBuffer);
                }

                    //or end -> keep current rpm

                calc_timer_segments();

                UART_Send(TS_PORT, "\rrpm: ");
                UART_Print_U(TS_PORT, Tuareg_simulator.pCrank_simulator->rpm, TYPE_U16, NO_PAD );


            }
            else
            {
                Tuareg_simulator.pCrank_simulator->sweep_counter++;

                //keep current rpm
                calc_timer_segments();
            }


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




