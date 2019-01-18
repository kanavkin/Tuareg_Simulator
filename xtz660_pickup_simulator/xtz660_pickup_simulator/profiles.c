/**
calculates the current engine rpm according to simulation cycle
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "types.h"
#include "simulator.h"



//standard cranking, rev up, rev down scenario
void run_rpm_profile_1()
{
    if(cycle_counter < 4)
    {
        calc_engine_timings(ENGINE_CRANKING_RPM);
    }
    else if((cycle_counter > 3) && (cycle_counter < 100))
    {
        if(engine_rpm < ENGINE_IDLE_RPM)
        {
            engine_rpm += (cycle_counter * 5);
        }
        else
        {
            engine_rpm= ENGINE_IDLE_RPM;
        }
        calc_engine_timings(engine_rpm);
    }
    else if((cycle_counter > 99) && (cycle_counter < 5000))
    {
        if(engine_rpm < ENGINE_MAX_RPM)
        {
            engine_rpm += 10;
        }
        else
        {
            engine_rpm= ENGINE_MAX_RPM;
        }
        calc_engine_timings(engine_rpm);
    }
    else if(cycle_counter > 4999)
    {
        if(engine_rpm > ENGINE_IDLE_RPM)
        {
            engine_rpm -= 18;
        }
        else
        {
            engine_rpm= ENGINE_IDLE_RPM;
        }
        calc_engine_timings(engine_rpm);
    }
}

//cranking simulation
void run_rpm_profile_2()
{
    if(cycle_counter < 3)
    {
        engine_rpm= ENGINE_CRANKING_RPM;
        calc_engine_timings(engine_rpm);
    }
    else
    {
        if(engine_rpm < ENGINE_IDLE_RPM)
        {
            engine_rpm += (cycle_counter * 5);
        }
        else
        {
            engine_rpm= ENGINE_IDLE_RPM;
        }
        calc_engine_timings(engine_rpm);
    }
}


//slow rev sweep idle .. max
void run_rpm_profile_3()
{
    if(cycle_counter < 100)
    {
        engine_rpm= ENGINE_IDLE_RPM;
        calc_engine_timings(engine_rpm);
    }
    else
    {
        if(engine_rpm < ENGINE_MAX_RPM)
        {
            engine_rpm++;
        }
        else
        {
            engine_rpm= ENGINE_MAX_RPM;
        }
        calc_engine_timings(engine_rpm);
    }
}


//faster rev sweep idle .. max
void run_rpm_profile_4()
{
    if(cycle_counter < 100)
    {
        engine_rpm= ENGINE_IDLE_RPM;
        calc_engine_timings(engine_rpm);
    }
    else
    {
        if(engine_rpm < ENGINE_MAX_RPM)
        {
            engine_rpm += 10;
        }
        else
        {
            engine_rpm= ENGINE_MAX_RPM;
        }
        calc_engine_timings(engine_rpm);
    }
}



/**
to be called from main program
*/
void run_rpm_profile(U8 profile_nr)
{
    switch(profile_nr)
            {
            case 0:

                //manual mode - use the new values by serial command
                calc_engine_timings(engine_rpm);
                break;

            case 1:
                run_rpm_profile_1();
                break;

            case 2:
                run_rpm_profile_2();
                break;

            case 3:
                run_rpm_profile_3();
                break;

            case 4:
                run_rpm_profile_4();
                break;

            default:
                run_rpm_profile_1();
                break;
            }

}
