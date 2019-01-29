/**



*/
#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"
#include "stm32_libs/boctok/boctok_types.h"

#include "types.h"
#include "ignition.h"
#include "decoder.h"
#include "scheduler.h"
#include "Tuareg.h"
#include "uart.h"
#include "table.h"

/**
characteristics hard coded to this tables!
*/
const U8 adv_table[]={0, 3, 3, 10, 12, 12, 12, 12, 12, 12, 14, 17, 24, 25, 26, 27, 28, \
29, 30, 35, 35, 36, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38};



/**
calculate the duration (in us) corresponding to an rotation angle
e.g. how long will it take the crank shaft to rotate by xx deg?
*/
U32 calc_rot_duration(U32 angle, U32 rpm)
{
    return (angle * 500000) / (rpm * 3);
}

/**
calculate the angle (in deg) that the crank shaft will rotate in
a given period at a given rpm
*/
U32 calc_rot_angle(U32 period_us, U32 rpm)
{
    return (3 * rpm * period_us) / 500000;
}



/**
calculates the position with the minimal possible delay time to fit
the advance for e.g. dwell and ignition
*/
void fit_position( U32 rpm, U32 advance, volatile engine_position_t * to_position, VU32 * to_delay)
{
    if(advance == POSITION_B2_ADVANCE)
    {
        // 0°
        * to_position= POSITION_B2;
        * to_delay= 0;
    }
    else if(advance <= POSITION_B1_ADVANCE)
    {
        // 1° - 10°
        * to_position= POSITION_B1;
        * to_delay= calc_rot_duration((POSITION_B1_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_B1_ADVANCE) && (advance <= POSITION_A2_ADVANCE))
    {
        // 11° - 60°
        * to_position= POSITION_A2;
        * to_delay= calc_rot_duration((POSITION_A2_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_A2_ADVANCE) && (advance <= POSITION_A1_ADVANCE))
    {
        // 61° - 100°
        * to_position= POSITION_A1;
        * to_delay= calc_rot_duration((POSITION_A1_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_A1_ADVANCE) && (advance <= POSITION_D2_ADVANCE))
    {
        // 101° - 185°
        * to_position= POSITION_D2;
        * to_delay= calc_rot_duration((POSITION_D2_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_D2_ADVANCE) && (advance <= POSITION_D1_ADVANCE))
    {
        // 186° - 190°
        * to_position= POSITION_D1;
        * to_delay= calc_rot_duration((POSITION_D1_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_D1_ADVANCE) && (advance <= POSITION_C2_ADVANCE))
    {
        // 191° - 275°
        * to_position= POSITION_C2;
        * to_delay= calc_rot_duration((POSITION_C2_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_C2_ADVANCE) && (advance <= POSITION_C1_ADVANCE))
    {
        // 276° - 280°
        * to_position= POSITION_C1;
        * to_delay= calc_rot_duration((POSITION_C1_ADVANCE - advance), rpm);
    }
    else if(advance > POSITION_C1_ADVANCE)
    {
        // 280° - 360°
        * to_position= POSITION_B2;
        * to_delay= calc_rot_duration((360 - advance), rpm);
    }
}



/**
calculates the ignition timing for the next engine cycle
at a given rpm (from ignition_timing_t) and writes it there

problem on low rpms:    if coil_on and coil_off trigger on the same engine_position AND
                        dwell < segment_duration no spark will be generated
                        due to scheduler allocation error
                        n_crit= d/(6*dwell)
                        (1666 rpm at 5ms dwell + 50° segment)
            solution:   do not use the scheduler for dwell timing,
                        turn the coil on right on engine position
                        (extending dwell)

problem on high rpms:   with a large dwell AND large ignition advance
                        coil_on event can leave the engine cycle (dwell advance > 360°)
                        *not feasible with 2 cylinders and 1 coil*
            solution:   clip dwell advance to 360°
                        (cutting dwell)

*/
void calc_ignition_timings(volatile ignition_timing_t * target_timing)
{
    if(target_timing->rpm > DYNAMIC_MIN_RPM)
    {
        /**
        calculate advance (off timing)
        */
        target_timing->ignition_advance= get_advance(target_timing->rpm);
        fit_position(target_timing->rpm, target_timing->ignition_advance, &target_timing->coil_off_pos, &target_timing->coil_off_timing );

        /**
        calculate dwell (on timing)
        */
        target_timing->dwell_advance= target_timing->ignition_advance + calc_rot_angle(DYNAMIC_DWELL_US, target_timing->rpm);

        /**
        clip dwell to crank cycle
        */
        if(target_timing->dwell_advance > 360)
        {
            target_timing->dwell_advance= 360;
        }

        fit_position(target_timing->rpm, target_timing->dwell_advance, &target_timing->coil_on_pos, &target_timing->coil_on_timing );

        /**
        account for scheduler allocation
        */
        if(target_timing->coil_on_pos == target_timing->coil_off_pos)
        {
            target_timing->coil_on_timing=0;
        }

    }
    else if(target_timing->rpm > LOWREV_MIN_RPM)
    {
        /**
        use fixed ignition triggers while idle
        */
        target_timing->coil_on_pos= LOWREV_DWELL_POSITION;
        target_timing->coil_off_pos= LOWREV_IGNITION_POSITION;
        target_timing->ignition_advance= LOWREV_IGNITION_ADVANCE;
        target_timing->coil_on_timing= 0;
        target_timing->coil_off_timing= 0;
    }
    else
    {
        /**
        use fixed ignition triggers while cranking
        */
        target_timing->coil_on_pos= CRANKING_DWELL_POSITION;
        target_timing->coil_off_pos= CRANKING_IGNITION_POSITION;
        target_timing->ignition_advance= CRANKING_IGNITION_ADVANCE;
        target_timing->coil_on_timing= 0;
        target_timing->coil_off_timing= 0;
    }
}



/**
    using
    -GPIOB-0 for ignition coil 1

    -use EXTI IRQ 2 for ignition timing
     recalculation after spark has fired

*/
void init_ignition(volatile ignition_timing_t * initial_timing)
{
    //clock
    RCC->APB2ENR |= RCC_APB2Periph_GPIOB;

    //coil
    GPIO_configure(GPIOB, 0, GPIO_OUT_PP_2MHZ);
    set_ign_ch1(OFF);

    //sw irq on exti line 3
    EXTI->IMR |= EXTI_IMR_MR3;

    //enable sw exti irq (prio 10)
    NVIC_SetPriority(EXTI3_IRQn, 10UL);
    NVIC_ClearPendingIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);

    /**
    provide initial ignition timing
    */
    initial_timing->rpm= 0;
    calc_ignition_timings(initial_timing);
}



void trigger_coil_by_timer(U32 delay_us, output_pin_t level)
{
    if(delay_us == 0)
    {
        // immediate trigger
        set_ign_ch1(level);
    }
    else
    {
        scheduler_set_channel(IGN_CH1, level, delay_us);
    }
}


