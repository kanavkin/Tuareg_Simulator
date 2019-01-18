#ifndef SPEED_H_INCLUDED
#define SPEED_H_INCLUDED

#include "types.h"
#include "ignition.h"
#include "sensors.h"

#define CRANK_ANGLE_MAX  720
#define CRANK_ANGLE_MAX_IGN  360
#define CRANK_ANGLE_MAX_INJ  360 // The number of crank degrees that the system track over. 360 for wasted / timed batch and 720 for sequential


/**
This is the maximum rpm that the ECU will attempt to run at.
It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be allowed to run.
Lower number gives better performance
*/
#define MAX_RPM 9000



/**
When the serial buffer is filled to greater than this threshold value, the serial processing operations will be performed more urgently in order to avoid it overflowing.
Serial buffer is 64 bytes long, so the threshold is set at half this as a reasonable figure
*/


//**************************************************************************************************
// Config section
#define engineSquirtsPerCycle 2 //Would be 1 for a 2 stroke
//**************************************************************************************************



/**
this defines bit masks
for the currentStatus
*/

//Define the load algorithm
#define LOAD_SOURCE_MAP         0
#define LOAD_SOURCE_TPS         1

//Define bit positions within engine variable
#define BIT_ENGINE_RUN      0   // Engine running
#define BIT_ENGINE_CRANK    1   // Engine cranking
#define BIT_ENGINE_ASE      2   // after start enrichment (ASE)
#define BIT_ENGINE_WARMUP   3   // Engine in warmup
#define BIT_ENGINE_ACC      4   // in acceleration mode (TPS accel)
#define BIT_ENGINE_DCC      5   // in deceleration mode
#define BIT_ENGINE_MAPACC   6   // MAP acceleration mode
#define BIT_ENGINE_MAPDCC   7   // MAP deceleration mode

//Define masks for Squirt
#define BIT_SQUIRT_INJ1          0  //inj1 Squirt
#define BIT_SQUIRT_INJ2          1  //inj2 Squirt
#define BIT_SQUIRT_INJ3          2  //inj3 Squirt
#define BIT_SQUIRT_INJ4          3  //inj4 Squirt
#define BIT_SQUIRT_DFCO          4 //Decelleration fuel cutoff
#define BIT_SQUIRT_BOOSTCUT      5  //Fuel component of MAP based boost cut out
#define BIT_SQUIRT_TOOTHLOG1READY 6  //Used to flag if tooth log 1 is ready
#define BIT_SQUIRT_TOOTHLOG2READY 7  //Used to flag if tooth log 2 is ready (Log is not currently used)

//Define masks for spark variable
#define BIT_SPARK_HLAUNCH         0  //Hard Launch indicator
#define BIT_SPARK_SLAUNCH         1  //Soft Launch indicator
#define BIT_SPARK_HRDLIM          2  //Hard limiter indicator
#define BIT_SPARK_SFTLIM          3  //Soft limiter indicator
#define BIT_SPARK_BOOSTCUT        4  //Spark component of MAP based boost cut out
#define BIT_SPARK_ERROR           5  // Error is detected
#define BIT_SPARK_IDLE            6  // idle on
#define BIT_SPARK_SYNC            7  // Whether engine has sync or not

#define BIT_SPARK2_FLATSH         0 //Flat shift hard cut
#define BIT_SPARK2_FLATSS         1 //Flat shift soft cut
#define BIT_SPARK2_UNUSED3        2
#define BIT_SPARK2_UNUSED4        3
#define BIT_SPARK2_UNUSED5        4
#define BIT_SPARK2_UNUSED6        5
#define BIT_SPARK2_UNUSED7        6
#define BIT_SPARK2_UNUSED8        7




typedef enum {

    TMODE_INIT,

    TMODE_LOWLEVEL      =0x00,
    TMODE_CONFIGLOAD    =0x01,
    TMODE_HWINIT        =0x02,
    TMODE_MIGRATION     =0x03,

    TMODE_LIMP          =0x30,
    TMODE_DIAG          =0x40,


    TMODE_CRASH         =0x50,
    TMODE_EMERGENCY     =0x51,

    TMODE_KILLED        =0xF0,

    TMODE_RUN,

    TMODE_STALLED,
    TMODE_CRANKING,
    TMODE_PERFORMING,
    TMODE_RUNNING           =0xFF,
    TMODE_WARMUP,
    TMODE_PRIMING

} tuareg_runmode_t;



typedef enum {

    TERROR_CONFIG       =0x01

} tuareg_error_t;






/**
The status struct contains the current values for all 'live' variables
In current version this is 64 bytes
*/
typedef struct _Tuareg_t {

    /**
    access to core components
    */
    volatile decoder_t * decoder;
    volatile sensor_interface_t * sensor_interface;
    volatile ignition_timing_t ignition_timing;

    //statemachine and health status
    tuareg_runmode_t Runmode;
    tuareg_error_t Errors;

    //sidestand, crash and run switch counter
    U8 run_switch_counter;
    U8 crash_switch_counter;




  //volatile ardbool hasSync;
  U16 RPM;
  //S32 longRPM;
  //S16 mapADC;
  //S16 baroADC;
  //S32 MAP; //Has to be a long for PID calcs (Boost control)
  //U8 baro; //Barometric pressure is simply the inital MAP reading, taken before the engine is running. Alternatively, can be taken from an external sensor
  //U8 TPS; //The current TPS reading (0% - 100%)
  //U8 TPSlast; //The previous TPS reading
  //U32 TPS_time; //The time the TPS sample was taken
  //U32 TPSlast_time; //The time the previous TPS sample was taken
  //U8 tpsADC; //0-255 byte representation of the TPS
  //U8 tpsDOT;
  VS16 rpmDOT;
  U8 VE;
  //U8 O2;
  //U8 O2_2;
  //S16 coolant;
  //S16 cltADC;
  //S16 IAT;
  //S16 iatADC;
  //S16 batADC;
  //S16 O2ADC;
  //S16 O2_2ADC;
  //S16 dwell;
  U8 dwellCorrection; //The amount of correction being applied to the dwell time.
  U8 battery10; //The current BRV in volts (multiplied by 10. Eg 12.5V = 125)
  S8 advance; //Signed 8 bit as advance can now go negative (ATDC)
  U8 corrections;
  U8 TAEamount; //The amount of accleration enrichment currently being applied
  U8 egoCorrection; //The amount of closed loop AFR enrichment currently being applied
  U8 wueCorrection; //The amount of warmup enrichment currently being applied
  U8 batCorrection; //The amount of battery voltage enrichment currently being applied
  U8 iatCorrection; //The amount of inlet air temperature adjustment currently being applied
  U8 launchCorrection; //The amount of correction being applied if launch control is active
  U8 flexCorrection; //Amount of correction being applied to compensate for ethanol content
  U8 flexIgnCorrection; //Amount of additional advance being applied based on flex
  U8 afrTarget;
  U8 idleDuty;
  ardbool fanOn; //Whether or not the fan is turned on
  VU8 ethanolPct; //Ethanol reading (if enabled). 0 = No ethanol, 100 = pure ethanol. Eg E85 = 85.
  U32 TAEEndTime; //The target end time used whenever TAE is turned on
  VU8 squirt;
  VU8 spark;
  VU8 spark2;
  U8 engine;
  U16 PW1; //In uS
  U16 PW2; //In uS
  U16 PW3; //In uS
  U16 PW4; //In uS
  VU8 runSecs; //Counter of seconds since cranking commenced (overflows at 255 obviously)
  VU8 secl; //Continous
  VU16 loopsPerSecond;
  ardbool launchingSoft; //True when in launch control soft limit mode
  ardbool launchingHard; //True when in launch control hard limit mode
  U16 freeRAM;
  U16 clutchEngagedRPM;
  ardbool flatShiftingHard;
  VU16 startRevolutions; //A counter for how many revolutions have been completed since sync was achieved.
  U16 boostTarget;
  U8 testOutputs;
  ardbool testActive;
  //U16 boostDuty; //Percentage value * 100 to give 2 points of precision
  //U8 idleLoad; //Either the current steps or current duty cycle for the idle control.

  //U16 canin[16]; //16bit raw value of selected canin data for channel 0-15
  //U8 current_caninchannel; //start off at channel 0  was U8 current_caninchannel= 0;

  //U16 crankRPM; //The actual cranking RPM limit. Saves us multiplying it everytime from the config page

} Tuareg_t;


/**
access to global Tuareg data
*/
extern volatile Tuareg_t Tuareg;



#endif // SPEED_H_INCLUDED
