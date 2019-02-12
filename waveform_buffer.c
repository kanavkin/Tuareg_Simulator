#include "stm32_libs/stm32f10x/stm32f10x.h"
#include "stm32_libs/boctok_types.h"

#include "waveform_buffer.h"

volatile waveform_generator_t Waveform_Generator;
volatile waveform_t Crank_waveform_buffer[CRANK_WAVEFORM_BUFFER_LENGTH];
volatile waveform_t Sensors_waveform_buffer[CRANK_WAVEFORM_BUFFER_LENGTH];


void reset_waveform_buffer(waveform_type_t Type)
{
    volatile waveform_t * Buffer;
    U32 Buffer_length;

    //management part
    if(Type == CRANK_WAVEFORM)
    {
        Buffer= &Crank_waveform_buffer[0];
        Buffer_length= CRANK_WAVEFORM_BUFFER_LENGTH;

        Waveform_Generator.crank_waveform_length =0;
        Waveform_Generator.crank_waveform_rdpointer =0;
        Waveform_Generator.crank_generator_state= GENERATOR_OFF;

    }
    else
    {
        Buffer= &Sensors_waveform_buffer[0];
        Buffer_length= SENSOR_WAVEFORM_BUFFER_LENGTH;

        Waveform_Generator.sensor_waveform_length =0;
        Waveform_Generator.sensor_waveform_rdpointer =0;

    }

    //content
    for(VU8 del=0; del < Buffer_length; del++)
    {
        Buffer[del].increment =0;
        Buffer[del].length =0;
    }
}


/**
calling function shall observe parameter range!
*/
void waveform_add(waveform_type_t Type, S32 Increment, U32 Length)
{
    //append to current waveform, as long it has not been started yet and it fits to buffer
    if((Type == CRANK_WAVEFORM) && (Waveform_Generator.crank_waveform_rdpointer == 0) && (Waveform_Generator.crank_waveform_length < CRANK_WAVEFORM_BUFFER_LENGTH -1))
    {
        Crank_waveform_buffer[Waveform_Generator.crank_waveform_length].increment= Increment;
        Crank_waveform_buffer[Waveform_Generator.crank_waveform_length].length= Length;
        Waveform_Generator.crank_waveform_length++;
    }
    else if((Type == SENSOR_WAVEFORM) && (Waveform_Generator.sensor_waveform_rdpointer == 0) && (Waveform_Generator.sensor_waveform_length < SENSOR_WAVEFORM_BUFFER_LENGTH -1))
    {
        Sensors_waveform_buffer[Waveform_Generator.sensor_waveform_length].increment= Increment;
        Sensors_waveform_buffer[Waveform_Generator.sensor_waveform_length].length= Length;
        Waveform_Generator.sensor_waveform_length++;
    }

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


//returns the new rpm to be simulated
U32 update_crank_generator(U32 Start_rpm)
{
    volatile waveform_t waveform;
    VU32 result;

    //continue simulation if already running
    if(Waveform_Generator.crank_generator_state == GENERATOR_ON)
    {
        //is the current waveform segment valid




        //try to get new waveform segment
        if( waveform_get(CRANK_WAVEFORM, &waveform))
        {

        }
        else
        {
            //no new waveform segment available
            Waveform_Generator.crank_generator_state= GENERATOR_OFF;
        }





    }
    else
    {
        //start new run







        Waveform_Generator.crank_generator_state= GENERATOR_ON;
    }











}



