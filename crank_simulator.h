#ifndef CRANK_SIMULATOR_H_INCLUDED
#define CRANK_SIMULATOR_H_INCLUDED

#include "stm32_libs/boctok_types.h"
#include "trigger_wheel_layout.h"

/**
when to enable cylinder identification sensor irq
*/
#define CYLINDER_SENSOR_DISA_POSITION CRK_POSITION_B1

#define TIMER_PRESCALER 287
#define TIMER_PERIOD_US 4


#define CRANK_PATTERN_MAXLEN 20
#define CRANK_MAX_RPM 25000

#define SIGNAL_LEVEL_KEY OFF
#define SIGNAL_LEVEL_STB ON


#define DEFAULT_SWEEP_START 100
#define DEFAULT_SWEEP_END 9000
#define DEFAULT_SWEEP_INCREMENT 100
#define DEFAULT_SWEEP_HOLD 10


#define CRANK_ACCEL_RPM_PER_SQSEC 2000
#define CRANK_MAX_ACCEL 100



typedef enum {

    XTZ660,
    XTZ750,

    ENGINE_TYPE_COUNT

} engine_type_t;



typedef struct _crank_simulator_t {

//current rpm
VU32 rpm;
VU32 crank_period_us;

//desired rpm
VU32 target_rpm;

//the numbers of crank shaft revolutions done
VU32 crank_turns;

// holds the timer values for the next crank turn
crank_position_table_t crank_timer_segments;

// holds the segments lengths for one crank turn (in deg)
crank_position_table_t crank_segments;

volatile crank_position_t crank_position;

//parameters for continuous mode
VU32 cont_rpm;

//parameters for sweep mode
VU32 sweep_start;
VU32 sweep_end;
VU32 sweep_increment;
VU32 sweep_hold;
VU32 sweep_counter;

//camshaft simulation: simplified cam model
crank_position_t cam_on_position;
crank_position_t cam_off_position;
//VU32 cam_offset_deg;
//VU32 cam_on_timing_us;
//VU32 cam_interval_deg;
//VU32 cam_duration_us;

/// TODO (oli#1#): this solution expects the cis signal occuring at a specific phase for all engine types!
volatile bool phase_cyl1_comp;


} crank_simulator_t;


extern volatile crank_simulator_t Crank_simulator;


void set_engine_type(engine_type_t new_engine);
void set_crank_rpm(VU32 Rpm);
void update_crank_rpm();
void set_target_rpm(VU32 Rpm);
void calc_timer_segments();
void calc_cam_parameters();
void start_crank_simulation();
void stop_crank_simulation();
volatile crank_simulator_t * init_crank_simulator();
VU16 pull_segment_timing();
void load_timer_compare(VU16 Compare);
void set_continuous_mode();
void set_sweep_mode();
#endif // CRANK_SIMULATOR_H_INCLUDED
