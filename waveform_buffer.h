#ifndef WAVEFORM_BUFFER_H_INCLUDED
#define WAVEFORM_BUFFER_H_INCLUDED

//buffer functions implemented for Length < 256
#define CRANK_WAVEFORM_BUFFER_LENGTH 100
#define SENSOR_WAVEFORM_BUFFER_LENGTH 100


typedef struct _waveform_t {

VU32 length;
VS32 increment;

} waveform_t ;


typedef struct _waveform_buffer_mgmt_t {

VU8 crank_waveform_length;
VU8 crank_waveform_rdpointer;
VU8 sensor_waveform_length;
VU8 sensor_waveform_rdpointer;


} waveform_buffer_mgmt_t ;


typedef enum {

    CRANK_WAVEFORM,
    SENSOR_WAVEFORM

} waveform_type;



void reset_waveform_buffer(waveform_type Type);
void waveform_add(waveform_type Type, S32 Increment, U32 Length);
U32 waveform_get(waveform_type Type, volatile waveform_t * Target_waveform);


#endif // WAVEFORM_BUFFER_H_INCLUDED
