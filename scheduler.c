/**
this scheduler provides the ignition and fuel subsystem
with a precise time base

it uses the 4 compare channels on timer 3 for this

timer resources:
    -ignition channel 1 -> compare channel 1
    -ignition channel 2 -> compare channel 2
    -fuel channel 1 -> compare channel 3
    -fuel channel 2 -> compare channel 4
*/
#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"

#include "types.h"
#include "scheduler.h"
#include "ignition.h"

volatile scheduler_t Scheduler;


/**
TODO
remove dummies!
*/
void set_fuel_ch1(output_pin_t level)
{

}

void set_fuel_ch2()
{

}


void init_scheduler()
{
    //clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // clear flags
    TIM3->SR= (U16) 0;

    //set prescaler
    TIM3->PSC= (U16) (SCHEDULER_PS - 1);

    //start timer counter
    TIM3->CR1 |= TIM_CR1_CEN;

    //enable timer 3 irq (prio 2)
    NVIC_SetPriority(TIM3_IRQn, 2UL );
    NVIC_ClearPendingIRQ(TIM3_IRQn);
    NVIC_EnableIRQ(TIM3_IRQn);
}


void scheduler_set_channel(scheduler_channel_t target_ch, output_pin_t action, U32 delay_us)
{

    U32 compare;
    U32 delay_ticks;
    U32 now;
    U32 remain;

    //safety check
    if(delay_us > SCHEDULER_MAX_PERIOD_US)
    {
        delay_us= SCHEDULER_MAX_PERIOD_US;
    }


    now= TIM3->CNT;

    //little correction to fit the desired delay better
    delay_ticks= delay_us / SCHEDULER_PERIOD_US +1;

    compare= now  + delay_ticks;

    switch(target_ch)
    {
        case IGN_CH1:

            TIM3->DIER &= (U16) ~TIM_DIER_CC1IE;

            Scheduler.ign_ch1_action= action;

            if(compare >= 0xFFFF)
            {
                /**
                the desired timeout will occur in the next timer cycle
                */
                remain= compare - 0xFFFF;

                if(remain < now)
                {
                    //compare already behind, hits after update
                    TIM3->CCMR1 &= ~TIM_CCMR1_OC1PE;
                }
                else
                {
                    //set new compare after update event
                    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;
                }

                TIM3->CCR1  = (U16) remain;

            }
            else
            {
                /**
                the desired timeout will occur in the current timer cycle
                */
                TIM3->CCMR1 &= ~TIM_CCMR1_OC1PE;
                TIM3->CCR1  = (U16) compare;

            }

            //clear pending flags and enable irq
            TIM3->SR    = (U16) ~TIM_SR_CC1IF;
            TIM3->DIER |= (U16) TIM_DIER_CC1IE;
            break;


        case IGN_CH2:

            TIM3->DIER &= (U16) ~TIM_DIER_CC2IE;

            Scheduler.ign_ch2_action= action;

            if(compare >= 0xFFFF)
            {
                /**
                the desired timeout will occur in the next timer cycle
                */
                remain= compare - 0xFFFF;

                if(remain < now)
                {
                    //compare already behind, hits after update
                    TIM3->CCMR1 &= ~TIM_CCMR1_OC2PE;
                }
                else
                {
                    //set new compare after update event
                    TIM3->CCMR1 |= TIM_CCMR1_OC2PE;
                }

                TIM3->CCR2  = (U16) remain;

            }
            else
            {
                /**
                the desired timeout will occur in the current timer cycle
                */
                TIM3->CCMR1 &= ~TIM_CCMR1_OC2PE;
                TIM3->CCR2  = (U16) compare;

            }

            //clear pending flags and enable irq
            TIM3->SR    = (U16) ~TIM_SR_CC2IF;
            TIM3->DIER |= (U16) TIM_DIER_CC2IE;
            break;


        case FUEL_CH1:

            TIM3->DIER &= (U16) ~TIM_DIER_CC3IE;

            Scheduler.fuel_ch1_action= action;

            if(compare >= 0xFFFF)
            {
                /**
                the desired timeout will occur in the next timer cycle
                */
                remain= compare - 0xFFFF;

                if(remain < now)
                {
                    //compare already behind, hits after update
                    TIM3->CCMR2 &= ~TIM_CCMR2_OC3PE;
                }
                else
                {
                    //set new compare after update event
                    TIM3->CCMR2 |= TIM_CCMR2_OC3PE;
                }

                TIM3->CCR3  = (U16) remain;

            }
            else
            {
                /**
                the desired timeout will occur in the current timer cycle
                */
                TIM3->CCMR2 &= ~TIM_CCMR2_OC3PE;
                TIM3->CCR3  = (U16) compare;

            }

            //clear pending flags and enable irq
            TIM3->SR    = (U16) ~TIM_SR_CC3IF;
            TIM3->DIER |= (U16) TIM_DIER_CC3IE;
            break;


        case FUEL_CH2:

            TIM3->DIER &= (U16) ~TIM_DIER_CC4IE;

            Scheduler.fuel_ch2_action= action;

            if(compare >= 0xFFFF)
            {
                /**
                the desired timeout will occur in the next timer cycle
                */
                remain= compare - 0xFFFF;

                if(remain < now)
                {
                    //compare already behind, hits after update
                    TIM3->CCMR2 &= ~TIM_CCMR2_OC4PE;
                }
                else
                {
                    //set new compare after update event
                    TIM3->CCMR2 |= TIM_CCMR2_OC4PE;
                }

                TIM3->CCR4  = (U16) remain;

            }
            else
            {
                /**
                the desired timeout will occur in the current timer cycle
                */
                TIM3->CCMR2 &= ~TIM_CCMR2_OC4PE;
                TIM3->CCR4  = (U16) compare;

            }

            //clear pending flags and enable irq
            TIM3->SR    = (U16) ~TIM_SR_CC4IF;
            TIM3->DIER |= (U16) TIM_DIER_CC4IE;
            break;

    default:
        break;

    }

}



void scheduler_reset_channel(scheduler_channel_t target_ch)
{
    switch(target_ch)
    {
        case IGN_CH1:

            TIM3->DIER &= (U16) ~TIM_DIER_CC1IE;
            TIM3->SR    = (U16) ~TIM_SR_CC1IF;
            break;

        case IGN_CH2:

            TIM3->DIER &= (U16) ~TIM_DIER_CC2IE;
            TIM3->SR    = (U16) ~TIM_SR_CC2IF;
            break;

        case FUEL_CH1:

            TIM3->DIER &= (U16) ~TIM_DIER_CC3IE;
            TIM3->SR    = (U16) ~TIM_SR_CC3IF;
            break;

        case FUEL_CH2:

            TIM3->DIER &= (U16) ~TIM_DIER_CC4IE;
            TIM3->SR    = (U16) ~TIM_SR_CC4IF;
            break;

        default:
            break;

    }
}





void TIM3_IRQHandler(void)
{
    if( TIM3->SR & TIM_SR_CC1IF)
    {
        /**
        ignition channel 1
        */
        set_ign_ch1(Scheduler.ign_ch1_action);

        //clear irq pending bit
        TIM3->SR= (U16) ~TIM_SR_CC1IF;

        //disable compare irq
        TIM3->DIER &= (U16) ~TIM_DIER_CC1IE;
    }


    if( TIM3->SR & TIM_SR_CC2IF)
    {
        /**
        ignition channel 2
        */
        set_ign_ch2(Scheduler.ign_ch2_action);

        //clear irq pending bit
        TIM3->SR= (U16) ~TIM_SR_CC2IF;

        //disable compare irq
        TIM3->DIER &= (U16) ~TIM_DIER_CC2IE;
    }

    if( TIM3->SR & TIM_SR_CC3IF)
    {
        /**
        fuel channel 1
        */
        set_fuel_ch1(Scheduler.fuel_ch1_action);

        //clear irq pending bit
        TIM3->SR= (U16) ~TIM_SR_CC3IF;

        //disable compare irq
        TIM3->DIER &= (U16) ~TIM_DIER_CC3IE;
    }

    if( TIM3->SR & TIM_SR_CC4IF)
    {
        /**
        fuel channel 2
        */
        set_fuel_ch2(Scheduler.ign_ch2_action);

        //clear irq pending bit
        TIM3->SR= (U16) ~TIM_SR_CC4IF;

        //disable compare irq
        TIM3->DIER &= (U16) ~TIM_DIER_CC4IE;
    }

}
