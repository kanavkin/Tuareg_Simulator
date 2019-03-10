#ifndef CRANK_SIMULATOR_H_INCLUDED
#define CRANK_SIMULATOR_H_INCLUDED

#include "stm32_libs/boctok_types.h"

/**
when to enable cylinder identification sensor irq
*/
#define CYLINDER_SENSOR_ENA_POSITION POSITION_B2
#define CYLINDER_SENSOR_DISA_POSITION POSITION_C1



#define CRANK_PATTERN_MAXLEN 20
#define CRANK_MAX_RPM 9500


typedef enum {

    XTZ660 =1,
    XTZ750 =2

} engine_type_t;



typedef struct _crank_simulator_t {

//simulated rpm
VU32 rpm;

//the numbers of crank shaft revolutions done
VU32 crank_turns;


// holds the timer values for the next crank turn
VU16 crank_timer_segments[CRANK_PATTERN_MAXLEN];

// holds the segments lengths for the next crank turn (in deg)
VU16 crank_segments[CRANK_PATTERN_MAXLEN];

VU8 crank_pattern_len;

VU8 crank_position;

//camshaft simulation
VU8 cam_on_angle;
VU8 cam_duration;


} crank_simulator_t;


void set_engine_type(engine_type_t new_engine);
void set_crank_rpm(U32 Rpm);
void recalc_timer_segments();
U32 get_simulator_rpm();
void start_crank_simulation(U32 Init_rpm);
void stop_crank_simulation();
volatile crank_simulator_t * init_crank_simulation();

#endif // CRANK_SIMULATOR_H_INCLUDED
