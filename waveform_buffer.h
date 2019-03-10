#ifndef WAVEFORM_BUFFER_H_INCLUDED
#define WAVEFORM_BUFFER_H_INCLUDED

#include "crank_simulator.h"


//buffer functions implemented for Length < 256
#define CRANK_WAVEFORM_BUFFER_LENGTH 100
#define SENSOR_WAVEFORM_BUFFER_LENGTH 100


typedef struct _waveform_t {

VU32 length;
VS32 increment;

} waveform_t ;



typedef struct _waveform_generator_t {

VU8 crank_waveform_length;
VU8 crank_waveform_rdpointer;
VU8 sensor_waveform_length;
VU8 sensor_waveform_rdpointer;

VU8 crank_generator_state;

} waveform_generator_t ;



typedef enum {

    CRANK_WAVEFORM,
    SENSOR_WAVEFORM

} waveform_type_t;



typedef enum {

    GENERATOR_ON,
    GENERATOR_OFF

} generator_state_t;


U32 start_crank_waveform_generator();
U32 update_crank_generator(VU32 * pTargetRpm);

void reset_waveform_buffer(waveform_type_t Type);
U32 waveform_add(waveform_type_t Type, S32 Increment, U32 Length);
U32 waveform_get(waveform_type_t Type, volatile waveform_t * Target_waveform);


#endif // WAVEFORM_BUFFER_H_INCLUDED
