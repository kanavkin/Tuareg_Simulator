#include "stm32_libs/stm32f10x/stm32f10x.h"
#include "stm32_libs/boctok_types.h"

#include "waveform_buffer.h"
#include "crank_simulator.h"

volatile waveform_generator_t Waveform_Generator;
volatile waveform_t Crank_waveform_buffer[CRANK_WAVEFORM_BUFFER_LENGTH];
volatile waveform_t Sensors_waveform_buffer[CRANK_WAVEFORM_BUFFER_LENGTH];


/**
how to use the crank waveform generator:

INIT
run reset_waveform_buffer() for the buffer to be initialised


LOAD
run waveform_add()

RUN
call start_crank_waveform_generator()
load the crank simulator with the returned rpm

then
(in the interrupt service routine when simulated crank revolution has finished)
call update_crank_generator()
reload the crank simulator with the returned rpm

STOP

*/





void reset_waveform_buffer(waveform_type_t Type)
{
    volatile waveform_t * pBuffer;
    U32 Buffer_length;

    //management part
    if(Type == CRANK_WAVEFORM)
    {
        pBuffer= &Crank_waveform_buffer[0];
        Buffer_length= CRANK_WAVEFORM_BUFFER_LENGTH;

        Waveform_Generator.crank_waveform_length =0;
        Waveform_Generator.crank_waveform_rdpointer =0;
        Waveform_Generator.crank_generator_state= GENERATOR_OFF;
    }
    else
    {
        pBuffer= &Sensors_waveform_buffer[0];
        Buffer_length= SENSOR_WAVEFORM_BUFFER_LENGTH;

        Waveform_Generator.sensor_waveform_length =0;
        Waveform_Generator.sensor_waveform_rdpointer =0;

    }

    //content
    for(VU8 del=0; del < Buffer_length; del++)
    {
        pBuffer[del].increment =0;
        pBuffer[del].length =0;
    }
}


/**
calling function shall observe parameter range!
*/
U32 waveform_add(waveform_type_t Type, S32 Increment, U32 Length)
{
    //append to current waveform, as long it has not been started yet and it fits to buffer
    if(Type == CRANK_WAVEFORM)
    {
        if((Waveform_Generator.crank_waveform_length < CRANK_WAVEFORM_BUFFER_LENGTH -1) && (Waveform_Generator.crank_generator_state == GENERATOR_OFF))
        {
            Crank_waveform_buffer[Waveform_Generator.crank_waveform_length].increment= Increment;
            Crank_waveform_buffer[Waveform_Generator.crank_waveform_length].length= Length;
            Waveform_Generator.crank_waveform_length++;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if(Type == SENSOR_WAVEFORM)
    {
        if((Waveform_Generator.sensor_waveform_rdpointer == 0) && (Waveform_Generator.sensor_waveform_length < SENSOR_WAVEFORM_BUFFER_LENGTH -1))
        {
            Sensors_waveform_buffer[Waveform_Generator.sensor_waveform_length].increment= Increment;
            Sensors_waveform_buffer[Waveform_Generator.sensor_waveform_length].length= Length;
            Waveform_Generator.sensor_waveform_length++;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else return 2;

}



/**
calling function shall observe parameter range!
*/
U32 waveform_get(waveform_type_t Type, volatile waveform_t * Target_waveform)
{
    if((Type == CRANK_WAVEFORM) && (Waveform_Generator.crank_waveform_rdpointer < Waveform_Generator.crank_waveform_length))
    {
        //copy over
        Target_waveform->increment= Crank_waveform_buffer[Waveform_Generator.crank_waveform_rdpointer].increment;
        Target_waveform->length= Crank_waveform_buffer[Waveform_Generator.crank_waveform_rdpointer].length;

        Waveform_Generator.crank_waveform_rdpointer++;
        return 0;
    }
    else if((Type == SENSOR_WAVEFORM) && (Waveform_Generator.sensor_waveform_rdpointer < Waveform_Generator.sensor_waveform_length))
    {
        //copy over
        Target_waveform->increment= Sensors_waveform_buffer[Waveform_Generator.sensor_waveform_rdpointer].increment;
        Target_waveform->length= Sensors_waveform_buffer[Waveform_Generator.sensor_waveform_rdpointer].length;

        Waveform_Generator.sensor_waveform_rdpointer++;
        return 0;
    }
    else
    {
        //error
        return 1;
    }

}


/**
read the new rpm to simulate from waveform buffer
returns 0 on success
*/
U32 update_crank_generator(VU32 * pTargetRpm)
{
    VS32 rpm_buffer;

    /**
    try to calculate the new rpm from the current waveform segment (when length > 0) or
    switch over to the next segment if possible
    */
    if(Crank_waveform_buffer[Waveform_Generator.crank_waveform_rdpointer].length == 0)
    {
        //segment has expired -> try to get the next segment

        //we can get a new segment if we are at the предпоследный segment
        if(Waveform_Generator.crank_waveform_rdpointer < Waveform_Generator.crank_waveform_length -1)
        {
            //proceed to the next segment
            Waveform_Generator.crank_waveform_rdpointer++;
        }
        else
        {
            //end of waveform
            Waveform_Generator.crank_generator_state= GENERATOR_OFF;
            return 1;
        }
    }

    /**
    now we know that our waveform segment is active
    */

    //remaining length shortened
    Crank_waveform_buffer[Waveform_Generator.crank_waveform_rdpointer].length--;


    //current waveform segment active
    rpm_buffer= *pTargetRpm;
    rpm_buffer += Crank_waveform_buffer[Waveform_Generator.crank_waveform_rdpointer].increment;

    //clamp rpm to positive values
    if(rpm_buffer < 0)
    {
        *pTargetRpm= 0;
    }
    else if(rpm_buffer > CRANK_MAX_RPM)
    {
        *pTargetRpm= CRANK_MAX_RPM;
    }
    else
    {
        *pTargetRpm= rpm_buffer;
    }

    return 0;

}



/**
returns 0 if successful
return 1 if no data
*/
U32 start_crank_waveform_generator()
{
    //if we have segments in buffer -> turn generator on
    if((Waveform_Generator.crank_generator_state != GENERATOR_ON) && (Waveform_Generator.crank_waveform_rdpointer < Waveform_Generator.crank_waveform_length))
    {
        Waveform_Generator.crank_generator_state= GENERATOR_ON;
        return 0;
    }
    else
    {
        Waveform_Generator.crank_generator_state= GENERATOR_OFF;
        return 1;
    }
}



