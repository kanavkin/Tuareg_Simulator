#ifndef SENSORS_H_INCLUDED
#define SENSORS_H_INCLUDED

#include "types.h"
#include "decoder.h"


/**
digital sensor channels:

PORTB4 -> DSENSOR_SPARE2
PORTC0 -> DSENSOR_SPARE1
PORTC2 -> DSENSOR_RUN
PORTC3 -> DESNSOR_CRASH
PORTC5 -> DSENSOR_DEBUG
*/



//use ADC1 for analog sensors
#define SENSOR_ADC ADC1

/**
ADC channels vs analog sensors

0   -> PORTA0  -> ASENSOR_O2
1   -> PORTA1  -> ASENSOR_TPS
2   -> PORTA2  -> ASENSOR_MAP
3   -> PORTA3  -> ASENSOR_IAT
4   -> PORTA4  -> ASENSOR_CLT
5   -> PORTA5  -> ASENSOR_VBAT
6   -> PORTA6  -> ASENSOR_KNOCK
7   -> PORTA7  -> ASENSOR_BARO
14  -> PORTC4  -> ASENSOR_SPARE
*/

/*
"wasted" ADC channels:
8  -> PORTB0    (used for Decoder - CRANK)
9  -> PORTB1    (used for Decoder - CAM)
10  -> PORTC0   (used for digital spare 1 sensor)
11  -> PORTC1   (used for VR spare channel)
12  -> PORTC2   (used for run switch)
13  -> PORTC3   (used for crash switch)
15  -> PORTC5   (used for debug switch)
*/

#define ASENSOR_O2_CH       ADC_Channel_0
#define ASENSOR_TPS_CH      ADC_Channel_1
#define ASENSOR_MAP_CH      ADC_Channel_2
#define ASENSOR_IAT_CH      ADC_Channel_3
#define ASENSOR_CLT_CH      ADC_Channel_4
#define ASENSOR_VBAT_CH     ADC_Channel_5
#define ASENSOR_KNOCK_CH    ADC_Channel_6
#define ASENSOR_BARO_CH     ADC_Channel_7
#define ASENSOR_SPARE_CH    ADC_Channel_14

// number of channels in the regular group for periodic conversion: O2, TPS, IAT, CLT, VBAT, KNOCK, BARO, SPARE
#define REGULAR_GROUP_LENGTH 8

//injected group for conversion on demand (in sync with engine process): MAP
#define INJECTED_GROUP_LENGTH 1

/**
generic values
TODO set more specific ones
TODO make values configurable (config page)

sensor timing considerations:
with an average buffer of 16 bit width we can fit up to 8
12 bit ADC values,
keep good balance between CPU load and accuracy!

T_upd= ASENSOR_x_AVG_THRES * 20ms * loop_count
*/
#define ASENSOR_MAP_MIN_THRES 302
#define ASENSOR_MAP_MAX_THRES 4095
#define ASENSOR_MAP_ERROR_THRES 0xFF
#define ASENSOR_MAP_AVG_THRES 16

#define ASENSOR_BARO_MIN_THRES 302
#define ASENSOR_BARO_MAX_THRES 4095
#define ASENSOR_BARO_ERROR_THRES 0xFF
#define ASENSOR_BARO_AVG_THRES 5

#define ASENSOR_TPS_MIN_THRES 1
#define ASENSOR_TPS_MAX_THRES 4095
#define ASENSOR_TPS_ERROR_THRES 0xFF
#define ASENSOR_TPS_AVG_THRES 5
#define DELTA_TPS_THRES 5

#define ASENSOR_O2_MIN_THRES 1
#define ASENSOR_O2_MAX_THRES 4095
#define ASENSOR_O2_ERROR_THRES 0xFF
#define ASENSOR_O2_AVG_THRES 5

#define ASENSOR_VBAT_MIN_THRES 1
#define ASENSOR_VBAT_MAX_THRES 4095
#define ASENSOR_VBAT_ERROR_THRES 0xFF
#define ASENSOR_VBAT_AVG_THRES 5

#define ASENSOR_IAT_MIN_THRES 1
#define ASENSOR_IAT_MAX_THRES 4095
#define ASENSOR_IAT_ERROR_THRES 0xFF
#define ASENSOR_IAT_AVG_THRES 5
#define IAT_OFFSET 40

#define ASENSOR_CLT_MIN_THRES 1
#define ASENSOR_CLT_MAX_THRES 4095
#define ASENSOR_CLT_ERROR_THRES 0xFF
#define ASENSOR_CLT_AVG_THRES 5
#define CLT_OFFSET 40

#define ASENSOR_SPARE_MIN_THRES 1
#define ASENSOR_SPARE_MAX_THRES 4095
#define ASENSOR_SPARE_ERROR_THRES 0xFF
#define ASENSOR_SPARE_AVG_THRES 5



/**
choose the ASENSOR_yy values so that they can address array elements in ADCBuffer[]: 0 ... (REGULAR_GROUP_LENGTH -1)
channels from injected group do not reserve space in the ADCBuffer[] -> they get the highest numbers
*/
typedef enum {

    //ADC based sensors

    //in regular group:
    ASENSOR_O2           =0x00,
    ASENSOR_TPS          =0x01,
    ASENSOR_IAT          =0x02,
    ASENSOR_CLT          =0x03,
    ASENSOR_VBAT         =0x04,
    ASENSOR_KNOCK        =0x05,
    ASENSOR_BARO         =0x06,
    ASENSOR_SPARE        =0x07,

    //internal ADC channels
    ASENSOR_ITEMP        =0x08,
    ASENSOR_IVREF        =0x09,

    //injected group
    ASENSOR_MAP          =0x10

} asensors_t;



/**
choose DSENSOR_xx values so that they can act as flags in a U8 (sensors.digital_sensors)
*/
typedef enum {

    //digital sensors
    DSENSOR_SPARE2       =0x01,
    DSENSOR_SPARE1       =0x02,
    DSENSOR_RUN          =0x04,
    DSENSOR_CRASH        =0x08,
    DSENSOR_DEBUG        =0x10

} dsensors_t;


typedef enum {

    //ADC based sensors
    ASENSOR_O2_ACT          =0x01,
    ASENSOR_TPS_ACT         =0x02,
    ASENSOR_IAT_ACT         =0x04,
    ASENSOR_CLT_ACT         =0x08,
    ASENSOR_VBAT_ACT        =0x10,
    ASENSOR_MAP_ACT         =0x20,
    ASENSOR_KNOCK_ACT       =0x40,
    ASENSOR_BARO_ACT        =0x80

} sensor_activity_t;


/**
sensor control

ADC measurement average values, its counters and error counters we address through average[ASENSOR_yy], average_counter[ASENSOR_yy] and error_counter[ASENSOR_yy]
only MAP sensor uses map_integrator and map_integrator_count for averaging.

when modifying sensors keep all indexes of regular group sensors below index of asensor_map
*/
typedef struct _sensor_interface_t {

    S16 ddt_TPS;

    sensor_activity_t active_sensors;

    U16 MAP;
    U16 BARO;
    U16 O2;
    U16 TPS;
    U16 IAT;
    U16 CLT;
    U16 VBAT;
    U16 Intake_Vacuum;
    U16 aSPARE;

    U8 digital_sensors;

    //working variables:

    U8 error_counter[REGULAR_GROUP_LENGTH + INJECTED_GROUP_LENGTH];

    U16 average[REGULAR_GROUP_LENGTH];
    U8 average_count[REGULAR_GROUP_LENGTH];

    U32 map_integrator;
    U8 map_integrator_count;

    U16 last_TPS;

    U8 loop_count;

} sensor_interface_t ;


volatile sensor_interface_t * init_sensors();
void read_digital_sensors();


//extern volatile sensor_interface_t Sensors;


#endif // SENSORS_H_INCLUDED
