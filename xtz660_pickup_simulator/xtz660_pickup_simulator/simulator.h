#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED

/**
trigger wheel geometry
*/
#define CRANK_SEGMENT_A 40
#define CRANK_SEGMENT_B 50
#define CRANK_SEGMENT_C 10
#define CRANK_SEGMENT_D 80
#define CRANK_SEGMENT_E 5
#define CRANK_SEGMENT_F 85
#define CRANK_SEGMENT_G 5
#define CRANK_SEGMENT_H 85

#define POSITION_C1_ADVANCE 280
#define POSITION_C2_ADVANCE 275
#define POSITION_D1_ADVANCE 190
#define POSITION_D2_ADVANCE 185
#define POSITION_A1_ADVANCE 100
#define POSITION_A2_ADVANCE 60
#define POSITION_B1_ADVANCE 10
#define POSITION_B2_ADVANCE 0


/**
engine rpm capabilities
*/
#define ENGINE_CRANKING_RPM 300
#define ENGINE_IDLE_RPM 1350
#define ENGINE_MAX_RPM 8500
#define SIMULATOR_MAX_RPM 20000

/**
crank sensor setup
logical state when a key triggers the sensor
*/
#define KEY_SIGNAL_POLARITY OFF
#define IDLE_SIGNAL ON


/**
ignition setup
*/
#define CRANKING_IGNITION_POSITION POSITION_B2
#define CRANKING_IGNITION_ADVANCE POSITION_B2_ADVANCE
#define LOWREV_IGNITION_POSITION POSITION_B1
#define LOWREV_IGNITION_ADVANCE POSITION_B1_ADVANCE
#define LOWREV_MIN_RPM 700
#define DYNAMIC_MIN_RPM 2500
#define CRANKING_DWELL_POSITION POSITION_A1
#define LOWREV_DWELL_POSITION POSITION_A1

extern volatile U32 cycle_counter;
extern volatile U16 engine_rpm;


U16 calc_segment_time(U8 length, U16 rpm);
U8 calc_advance_time(U16 angle, U16 rpm);

U8 get_advance(U16 rpm);
U16 get_dwell(U16 rpm);

void fit_position(volatile U16 rpm, volatile U16 advance, volatile U8 * to_position, volatile U8 * to_timing);
void calc_engine_timings(U16 rpm);

void set_simulator_pin(U8 level);
void set_trigger_pin(U8 level);



#endif // SIMULATOR_H_INCLUDED
