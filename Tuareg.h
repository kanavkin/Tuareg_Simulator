#ifndef SPEED_H_INCLUDED
#define SPEED_H_INCLUDED

#include "stm32_libs/boctok_types.h"
#include "types.h"
#include "ignition.h"
#include "sensors.h"
#include "crank_simulator.h"



/**
This is the maximum rpm that the simulator will run at
*/
#define MAX_RPM 25000
#define DEFAULT_RPM 1350


typedef enum {

    SMODE_STOP,
    SMODE_CONT,
    SMODE_WAVEFORM,
    SMODE_SWEEP

} simulator_mode_t;




/**
T
*/
typedef struct _Tuareg_simulator_t {

    /**
    access to core components
    */
    volatile crank_simulator_t * pCrank_simulator;

    engine_type_t simulated_engine;

    volatile simulator_mode_t crank_simulator_mode;


} Tuareg_simulator_t;


/**
access to global Tuareg data
*/
extern volatile Tuareg_simulator_t Tuareg_simulator;



#endif // SPEED_H_INCLUDED
