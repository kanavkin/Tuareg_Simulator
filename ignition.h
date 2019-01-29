#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED

#include "stm32_libs/boctok/stm32_gpio.h"



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






U32 get_advance(U32 rpm);
U32 calc_rot_duration(U32 angle, U32 rpm);
U32 calc_rot_angle(U32 period_us, U32 rpm);



void trigger_coil_by_timer(U32 delay_us, output_pin_t level);





#endif // SIMULATOR_H_INCLUDED
