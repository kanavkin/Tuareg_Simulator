#include "stm32_libs/stm32f10x/stm32f10x.h"
#include "stm32_libs/stm32f10x/boctok/stm32f10x_gpio_boctok.h"
#include "stm32_libs/boctok_types.h"

#include "crank_simulator.h"
#include "Tuareg.h"
#include "base_calc.h"
#include "uart.h"
#include "conversion.h"

volatile crank_simulator_t Crank_simulator;



/**
crank signal on GPIOD0
*/
void set_crank_pin(output_pin_t level)
{
    gpio_set_pin(GPIOB, 10, level);
}



/**
cam signal on GPIOD1
*/
void set_cam_pin(output_pin_t level)
{
    gpio_set_pin(GPIOB, 11, level);
}



/**
helper function
stores the given rpm value to simulator and calculate the timer segments
*/
void set_crank_rpm(VU32 Rpm)
{
    if((Rpm > 0) && (Rpm <= CRANK_MAX_RPM))
    {
        Crank_simulator.rpm= Rpm;
        Crank_simulator.crank_period_us= calc_period_us(Rpm);
    }
    else
    {
        Crank_simulator.rpm= DEFAULT_RPM;
        Crank_simulator.crank_period_us= calc_period_us(DEFAULT_RPM);

        UART_Send(TS_PORT, "\r\nclipped crank rpm to max value!\r\n");
    }

}

/**
helper function
stores the given rpm value to simulator and calculate the timer segments
*/
void set_continuous_mode()
{
    if(Tuareg_simulator.crank_simulator_mode != SMODE_CONT)
    {
        stop_crank_simulation();

        Tuareg_simulator.crank_simulator_mode= SMODE_CONT;

        //check parameters
        if((Tuareg_simulator.pCrank_simulator->cont_rpm > MAX_RPM) || (Tuareg_simulator.pCrank_simulator->cont_rpm == 0))
        {
            Tuareg_simulator.pCrank_simulator->cont_rpm= DEFAULT_RPM;
        }

        set_crank_rpm(Tuareg_simulator.pCrank_simulator->cont_rpm);

        calc_timer_segments();

        UART_Send(TS_PORT, "\r\n starting continuous mode rpm: ");
        UART_Print_U(TS_PORT, Crank_simulator.rpm, TYPE_U16, NO_PAD);

        start_crank_simulation();
    }

}

/**
helper function
stores the given rpm value to simulator and calculate the timer segments
*/
void set_sweep_mode()
{
    if(Tuareg_simulator.crank_simulator_mode != SMODE_SWEEP)
    {
        stop_crank_simulation();

        Tuareg_simulator.crank_simulator_mode= SMODE_SWEEP;

        //init sweep parameters
        Tuareg_simulator.pCrank_simulator->sweep_counter =0;

        //check other sweep parameters
        if((Tuareg_simulator.pCrank_simulator->sweep_start > MAX_RPM) || (Tuareg_simulator.pCrank_simulator->sweep_start == 0))
        {
            Tuareg_simulator.pCrank_simulator->sweep_start= DEFAULT_SWEEP_START;
        }

        if((Tuareg_simulator.pCrank_simulator->sweep_end > MAX_RPM) || (Tuareg_simulator.pCrank_simulator->sweep_end == 0))
        {
            Tuareg_simulator.pCrank_simulator->sweep_end= DEFAULT_SWEEP_END;
        }

        if((Tuareg_simulator.pCrank_simulator->sweep_increment > MAX_RPM) || (Tuareg_simulator.pCrank_simulator->sweep_increment == 0))
        {
            Tuareg_simulator.pCrank_simulator->sweep_increment= DEFAULT_SWEEP_INCREMENT;
        }

        if((Tuareg_simulator.pCrank_simulator->sweep_hold > MAX_RPM) || (Tuareg_simulator.pCrank_simulator->sweep_hold == 0))
        {
            Tuareg_simulator.pCrank_simulator->sweep_hold= DEFAULT_SWEEP_HOLD;
        }

        UART_Send(TS_PORT, "\r\n starting sweep mode\r\nstart, end, inc, hld: ");
        UART_Print_U(TS_PORT, Crank_simulator.sweep_start, TYPE_U16, NO_PAD);
        UART_Print_U(TS_PORT, Crank_simulator.sweep_end, TYPE_U16, NO_PAD);
        UART_Print_U(TS_PORT, Crank_simulator.sweep_increment, TYPE_U16, NO_PAD);
        UART_Print_U(TS_PORT, Crank_simulator.sweep_hold, TYPE_U16, NO_PAD);
        UART_Send(TS_PORT, "\r\n");

        //start simulation
        set_crank_rpm(Tuareg_simulator.pCrank_simulator->sweep_start);

        calc_timer_segments();

        start_crank_simulation();

    }
}



/**
helper function
stores the given rpm value to simulator and calculate the timer segments
*/
void calc_timer_segments()
{
    for(VU32 segment=0; segment < CRK_POSITION_COUNT; segment++)
    {
        //t (in us) := 166667 * d (in °) / n (in rpm)
        Crank_simulator.crank_timer_segments.a_deg[segment]= calc_rot_duration_us(Crank_simulator.crank_segments.a_deg[segment], Crank_simulator.crank_period_us) / TIMER_PERIOD_US;
    }
}



/**
returns the duration for the next segment (following the current crank position) to simulate and deletes it from the buffer

-> can be called only once for each segment
*/
VU16 pull_segment_timing()
{
    VU16 duration;
    volatile crank_position_t position;

    position= Crank_simulator.crank_position;

    if( position < CRK_POSITION_COUNT)
    {
        duration= Crank_simulator.crank_timer_segments.a_deg[position];
        Crank_simulator.crank_timer_segments.a_deg[position] =0;

        return duration;
    }
    else
    {
        return 0;
    }
}



void set_engine_type(engine_type_t new_engine)
{
    if(new_engine == XTZ750)
    {
        //set up crank pattern
        Crank_simulator.crank_segments.a_deg[CRK_POSITION_A1]= XTZ750_POSITION_A2_ANGLE;
        Crank_simulator.crank_segments.a_deg[CRK_POSITION_A2]= XTZ750_POSITION_B1_ANGLE - XTZ750_POSITION_A2_ANGLE ;

        Crank_simulator.crank_segments.a_deg[CRK_POSITION_B1]= XTZ750_POSITION_B2_ANGLE - XTZ750_POSITION_B1_ANGLE;
        Crank_simulator.crank_segments.a_deg[CRK_POSITION_B2]= XTZ750_POSITION_C1_ANGLE - XTZ750_POSITION_B2_ANGLE;

        Crank_simulator.crank_segments.a_deg[CRK_POSITION_C1]= XTZ750_POSITION_C2_ANGLE - XTZ750_POSITION_C1_ANGLE;
        Crank_simulator.crank_segments.a_deg[CRK_POSITION_C2]= XTZ750_POSITION_D1_ANGLE - XTZ750_POSITION_C2_ANGLE;

        Crank_simulator.crank_segments.a_deg[CRK_POSITION_D1]= XTZ750_POSITION_D2_ANGLE - XTZ750_POSITION_D1_ANGLE;
        Crank_simulator.crank_segments.a_deg[CRK_POSITION_D2]= 360 - XTZ750_POSITION_D2_ANGLE;

        Tuareg_simulator.simulated_engine= XTZ750;

    }
    else if(new_engine == XTZ660)
    {
        //set up crank pattern
        Crank_simulator.crank_segments.a_deg[0]= 40;
        Crank_simulator.crank_segments.a_deg[1]= 50;

        Crank_simulator.crank_segments.a_deg[2]= 10;
        Crank_simulator.crank_segments.a_deg[3]= 80;

        Crank_simulator.crank_segments.a_deg[4]= 5;
        Crank_simulator.crank_segments.a_deg[5]= 85;

        Crank_simulator.crank_segments.a_deg[6]= 5;
        Crank_simulator.crank_segments.a_deg[7]= 85;

        Tuareg_simulator.simulated_engine= XTZ660;

    }


    /*
    TODO
    set up cam timing
    */

}



/**
use 16 bit TIM2 for crank pickup signal simulation
precondition: fill crank_timer_segments[] -> run calc_crank_timing()
*/
void start_crank_simulation()
{
    VU16 Segment_buffer;

    //first revolution
    Crank_simulator.crank_position= CRK_POSITION_A1;
    Crank_simulator.crank_turns =0;

    //load output compare value for the first pattern
    Segment_buffer= pull_segment_timing();

    //set the desired key signal level, irq will toggle it
    set_crank_pin(SIGNAL_LEVEL_KEY);

    if(Segment_buffer > 0)
    {
        load_timer_compare(Segment_buffer);

        //set prescaler for T= 1us
        TIM2->PSC= (U16) TIMER_PRESCALER;

        //start timer counter
        TIM2->CR1 |= TIM_CR1_CEN;
    }
    else
    {
        //error
        stop_crank_simulation();
    }

}


/**
load the segment duration to timer compare register and sets the timer up
*/
void load_timer_compare(VU16 Compare)
{
    //reset timer
    TIM2->CNT= (U16) 0;

    //set new timer compare value
    TIM2->CCR1= (U16) Compare;

    //generate update event to write new prescaler and compare value
    TIM2->EGR = TIM_EGR_UG;

    //enable compare 1 event
    TIM2->DIER |= TIM_DIER_CC1IE;
}




void stop_crank_simulation()
{
    //disable timer
    TIM2->CR1 &= ~TIM_CR1_CEN;

    set_crank_pin(SIGNAL_LEVEL_STB);

    Tuareg_simulator.crank_simulator_mode= SMODE_STOP;

    //reset variables
    Crank_simulator.crank_position =0;
    Crank_simulator.crank_turns =0;
    Crank_simulator.rpm =0;
}



/**
    using
    - GPIOD0 for crank simulation
    - GPIOD1 for cam simulation
*/
volatile crank_simulator_t * init_crank_simulator()
{
    //interface variables
    Crank_simulator.crank_position =0;
    Crank_simulator.crank_turns =0;

    //clock tree setup
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    //set output mode for crank and cylinder identification sensor signal
    GPIO_configure(GPIOB, 10, GPIO_OUT_PP_2MHZ);
    GPIO_configure(GPIOB, 11, GPIO_OUT_PP_2MHZ);

    //sw irq on EXTI line 2
    EXTI->IMR |= EXTI_IMR_MR2;

    //enable timer 2 compare 1 irq (prio 2)
    NVIC_SetPriority(TIM2_IRQn, 2UL );
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);

    //enable sw EXTI irq (prio 1)
    NVIC_SetPriority(EXTI2_IRQn, 1UL);
    NVIC_ClearPendingIRQ(EXTI2_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);


    //all done
    return &Crank_simulator;
}





/******************************************************************************************************************************
Timer 2 compare event 1 - simulator control - a segment end has been reached
 ******************************************************************************************************************************/
void TIM2_IRQHandler(void)
{
    VU16 Segment_buffer;

    //compare event
    if( TIM2->SR & TIM_SR_CC1IF)
    {
        TIM2->SR= (U16) ~TIM_SR_CC1IF;

        //apply new simulator pin level
        set_crank_pin(TOGGLE);

        //crank pattern segment duration expired -> new engine position reached
        Crank_simulator.crank_position++;

        //turn crank over if at pattern end -> would result in illegal array index, indices from [0 .. crank_pattern_len-1]
        if(Crank_simulator.crank_position == CRK_POSITION_COUNT)
        {
            /**
            a full crank revolution has been simulated
            */
            Crank_simulator.crank_position =0;
            Crank_simulator.crank_turns++;
        }

        /**
        reload timer -> the array index of the next segment always matches the position number we just reached
        fresh timer segments should have been generated after last segment load
        */
        Segment_buffer= pull_segment_timing();

        if(Segment_buffer > 0)
        {
            load_timer_compare(Segment_buffer);

            /**
            trigger sw irq 2 when loading last segment -> timing recalculation
            (this will fill crank_timer_segments[] with new values)
            */
            if(Crank_simulator.crank_position == (CRK_POSITION_COUNT -1))
            {
                EXTI->SWIER= EXTI_SWIER_SWIER2;
            }

        }
        else
        {
            /**
            empty crank_timer_segments -> shut down crank simulator (triggers sw irq)
            */
            stop_crank_simulation();
        }

    }



    /**
    handle update event
    these have never been enabled here!!!
    ~every 65 ms
    */
    if( TIM2->SR & TIM_IT_Update)
    {
        TIM2->SR = (U16) ~TIM_IT_Update;
    }

}



