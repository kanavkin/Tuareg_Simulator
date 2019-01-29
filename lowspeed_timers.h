#ifndef TIMERS_H
#define TIMERS_H

#include "stm32_libs/boctok/boctok_types.h"

#define BIT_TIMER_1HZ             0x01
#define BIT_TIMER_4HZ             0x02
#define BIT_TIMER_10HZ            0x04
#define BIT_TIMER_15HZ            0x08
#define BIT_TIMER_30HZ            0x10
#define BIT_TIMER_50HZ            0x20

extern VU32 ls_timer;
extern VU32 system_time;

void init_lowspeed_timers();

#endif // TIMERS_H
