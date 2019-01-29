#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"
#include "stm32_libs/boctok/boctok_types.h"

#include "crank_simulator.h"
#include "debug.h"



/**
strange behavior:
crank signal generation works properly while/after debug session,
after a system reset it works 69x faster

but sometimes it works after reset

*/



volatile crank_simulator_t Crank_simulator;



/**
crank signal on GPIOD0
*/
void set_crank_pin(output_pin_t level)
{
    gpio_set_pin(GPIOD, 0, level);
}



/**
cam signal on GPIOD1
*/
void set_cam_pin(output_pin_t level)
{
    gpio_set_pin(GPIOD, 1, level);
}



/**
use 16 bit TIM2 for crank pickup signal simulation
*/
void start_crank_simulation()
{
    VU32 Segment_buffer;

    //set prescaler for T= 1us
    TIM2->PSC= (U16) 71;

    //first revolution
    Crank_simulator.crank_turns =0;

    //load output compare value for the first pattern
    Segment_buffer= Crank_simulator.crank_timer_segments[0];
    //DEBUG
    //Segment_buffer= 100;

    Crank_simulator.crank_timer_segments[0] =0;

    if(Segment_buffer > 0)
    {
        //reset timer
        TIM2->CNT= (U16) 0;

        //set new timer compare value
        TIM2->CCR1= (U16) Segment_buffer;

        //crank signal first pattern start with high level
        set_crank_pin(ON);

        //enable compare 1 event
        TIM2->DIER |= TIM_DIER_CC1IE;

        //start timer counter
        TIM2->CR1 |= TIM_CR1_CEN;

        /**
        it has been shown that enabling the timer fires up the irq flags in SR (0x40000010 := 0x1f)
        so clear the flags!!!
        */
        TIM2->SR= (U16) 0;
    }
    else
    {
        //error
        stop_crank_simulation();
    }

}


void stop_crank_simulation()
{
    //disable timer
    TIM2->CR1 &= ~TIM_CR1_CEN;

    set_crank_pin(OFF);

    //reset variables
    Crank_simulator.crank_position =0;
    Crank_simulator.crank_turns =0;

    //trigger the sw irq 2 for error indication
    EXTI->SWIER= EXTI_SWIER_SWIER2;
}



/**
    using
    - GPIOD0 for crank simulation
    - GPIOD1 for cam simulation
*/
volatile crank_simulator_t * init_crank_simulation()
{
    //interface variables
    Crank_simulator.crank_position =0;
    Crank_simulator.crank_turns =0;

    //clock tree setup
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPDEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    //set output mode for crank and cylinder identification sensor signal
    GPIO_configure(GPIOD, 0, GPIO_OUT_PP_2MHZ);
    GPIO_configure(GPIOD, 1, GPIO_OUT_PP_2MHZ);

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
    VU32 Segment_buffer;

    //compare event
    if( TIM2->SR & TIM_IT_CC1)
    {
        TIM2->SR = (U16) ~TIM_IT_CC1;

        //crank pattern segment duration expired -> new engine position reached
        Crank_simulator.crank_position++;

        //turn crank over if at pattern end -> would result in illegal array index, indices from [0 .. crank_pattern_len-1]
        if(Crank_simulator.crank_position == Crank_simulator.crank_pattern_len)
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
        Segment_buffer=Crank_simulator.crank_timer_segments[Crank_simulator.crank_position];
        //DEBUG
        //Segment_buffer= 4938;

        if(Segment_buffer > 0)
        {
            //mask compare irq
            TIM2->DIER &= ~TIM_DIER_CC1IE;

            //set new timer compare value
            TIM2->CCR1= (U16) Segment_buffer;

            //reset timer
            TIM2->CNT= (U16) 0;

            //delete flags
            TIM2->SR= (U16) 0;

            //mask compare irq
            TIM2->DIER |= TIM_DIER_CC1IE;

            //delete old value
            Crank_simulator.crank_timer_segments[Crank_simulator.crank_position] =0;

            //trigger the sw irq 2 for timing recalculation, when the last segment has been loaded
            if(Crank_simulator.crank_position == (Crank_simulator.crank_pattern_len -1))
            {
                EXTI->SWIER= EXTI_SWIER_SWIER2;
            }

            //apply new simulator pin level
            set_crank_pin(TOGGLE);

        }
        else
        {
            //error -> shut down crank simulator (triggers sw irq)
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



