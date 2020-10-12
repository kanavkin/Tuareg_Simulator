#ifndef ROTATIONCALC_H_INCLUDED
#define ROTATIONCALC_H_INCLUDED

#include "stm32_libs/boctok_types.h"


U32 calc_rot_duration_us(U32 Angle_deg, U32 Period_us);
U32 calc_rot_angle_deg(U32 Interval_us, U32 Period_us);
U32 calc_rpm(U32 Period_us);
U32 calc_period_us(U32 Rpm);

void sub_VU32(VU32 * pMin, VU32 Subtr);
#endif // ROTATIONCALC_H_INCLUDED
