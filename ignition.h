#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED

/**
trigger wheel geometry
*/
#define POSITION_C1_ADVANCE 280
#define POSITION_C2_ADVANCE 275
#define POSITION_D1_ADVANCE 190
#define POSITION_D2_ADVANCE 185
#define POSITION_A1_ADVANCE 100
#define POSITION_A2_ADVANCE 60
#define POSITION_B1_ADVANCE 10
#define POSITION_B2_ADVANCE 0


/**
crank sensor setup
logical state when a key triggers the sensor
*/
#define KEY_SIGNAL_POLARITY OFF
#define IDLE_SIGNAL ON


/**
ignition timing setup

remember that there will be a significant voltage drop
while cranking -> needs longer dwell!
    (D2..B2 is about 180° dwell)

adjust DYNAMIC_MIN_RPM to a value higher than your
desired idle rpm if you want fixed ignition operation there
(rough idling, ...)
*/
#define CRANKING_DWELL_POSITION POSITION_D2
#define CRANKING_IGNITION_POSITION POSITION_B2
#define CRANKING_IGNITION_ADVANCE POSITION_B2_ADVANCE

#define LOWREV_DWELL_POSITION POSITION_A2
#define LOWREV_IGNITION_POSITION POSITION_B1
#define LOWREV_IGNITION_ADVANCE POSITION_B1_ADVANCE
#define LOWREV_MIN_RPM 700

#define DYNAMIC_MIN_RPM 1000
#define DYNAMIC_DWELL_US 5000


#include "decoder.h"


typedef struct _ignition_timing_t {

    U32 rpm;
    U32 ignition_advance;
    U32 dwell_advance;
    U32 coil_on_timing;
    U32 coil_off_timing;
    engine_position_t coil_on_pos;
    engine_position_t coil_off_pos;

} ignition_timing_t;



U32 get_advance(U32 rpm);
U32 calc_rot_duration(U32 angle, U32 rpm);
U32 calc_rot_angle(U32 period_us, U32 rpm);

void init_ignition(volatile ignition_timing_t * initial_timing);

void fit_position( U32 rpm, U32 advance, volatile engine_position_t * to_position, VU32 * to_delay);
void calc_ignition_timings(volatile ignition_timing_t * target_timing);

void set_ign_ch1(output_pin_t level);
void set_ign_ch2(output_pin_t level);

void trigger_coil_by_timer(U32 delay_us, output_pin_t level);





#endif // SIMULATOR_H_INCLUDED
