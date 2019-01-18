#ifndef CONFIG_PAGES_H_INCLUDED
#define CONFIG_PAGES_H_INCLUDED

#include "types.h"


#define INJ_PAIRED          0
#define INJ_SEMISEQUENTIAL  1
#define INJ_BANKED          2
#define INJ_SEQUENTIAL      3

#define IGN_MODE_WASTED     0
#define IGN_MODE_SINGLE     1
#define IGN_MODE_WASTEDCOP  2
#define IGN_MODE_SEQUENTIAL 3
#define IGN_MODE_ROTARY     4

#define ROTARY_IGN_FC       0
#define ROTARY_IGN_FD       1
#define ROTARY_IGN_RX8      2

#define BOOST_MODE_SIMPLE   0
#define BOOST_MODE_FULL     1




#define SIZE_BYTE   8
#define SIZE_INT    16

#define EVEN_FIRE         0
#define ODD_FIRE          1



/**
Page 1 of the config - See the ini file for further reference
This mostly covers off variables that are required for fuel
*/
typedef struct _configPage1_t_ {

  S8 flexBoostLow; //Must be signed to allow for negatives
  U8 flexBoostHigh;
  U8 asePct;  //Afterstart enrichment (%)
  U8 aseCount; //Afterstart enrichment cycles. This is the number of ignition cycles that the afterstart enrichment % lasts for
  U8 wueValues[10]; //Warm up enrichment array (10 bytes)
  U8 crankingPct; //Cranking enrichment
  U8 pinMapping; // The board / ping mapping to be used
  U8 tachoPin : 6; //Custom pin setting for tacho output
  U8 tachoDiv : 2; //Whether to change the tacho speed
  U8 tdePct; // TPS deceleration (%)
  U8 taeColdA;
  U8 tpsThresh;
  U8 taeTime;

  //Display config bits
  U8 displayType : 3; //21
  U8 display1 : 3;
  U8 display2 : 2;

  U8 display3 : 3;    //22
  U8 display4 : 2;
  U8 display5 : 3;

  U8 displayB1 : 4;   //23
  U8 displayB2 : 4;

  U8 reqFuel;       //24
  U8 divider;
  U8 injTiming : 1;
  U8 multiplyMAP : 1;
  U8 includeAFR : 1;
  U8 unused26 : 4;
  U8 indInjAng : 1;
  U8 injOpen; //Injector opening time (ms * 10)
  U16 inj1Ang;
  U16 inj2Ang;
  U16 inj3Ang;
  U16 inj4Ang;

  //config1 in ini
  U8 mapSample : 2;
  U8 strokes : 1;
  U8 injType : 1;
  U8 nCylinders : 4; //Number of cylinders

  //config2 in ini
  U8 cltType1 : 2;
  U8 matType1 : 2;
  U8 nInjectors : 4; //Number of injectors


  //config3 in ini
  U8 engineType : 1;
  U8 flexEnabled : 1;
  U8 algorithm : 1; //"Speed Density", "Alpha-N"
  U8 baroCorr : 1;
  U8 injLayout : 2;
  U8 perToothIgn : 1;
  U8 dfcoEnabled : 1; //Whether or not DFCO is turned on

  U8 primePulse;
  U8 dutyLim;
  U8 flexFreqLow; //Lowest valid frequency reading from the flex sensor
  U8 flexFreqHigh; //Highest valid frequency reading from the flex sensor

  U8 boostMaxDuty;
  U8 tpsMin;
  U8 tpsMax;
  S8 mapMin; //Must be signed
  U16 mapMax;
  U8 fpPrime; //Time (In seconds) that the fuel pump should be primed for on power up
  U8 stoich;
  U16 oddfire2; //The ATDC angle of channel 2 for oddfire
  U16 oddfire3; //The ATDC angle of channel 3 for oddfire
  U16 oddfire4; //The ATDC angle of channel 4 for oddfire
  U8 flexFuelLow; //Fuel % to be used for the lowest ethanol reading (Typically 100%)
  U8 flexFuelHigh; //Fuel % to be used for the highest ethanol reading (Typically 163%)
  U8 flexAdvLow; //Additional advance (in degrees) at lowest ethanol reading (Typically 0)
  U8 flexAdvHigh; //Additional advance (in degrees) at highest ethanol reading (Varies, usually 10-20)

  U8 iacCLminDuty;
  U8 iacCLmaxDuty;
  U8 boostMinDuty;

} configPage1_t ;


/**
Page 2 of the config - See the ini file for further reference
This mostly covers off variables that are required for ignition
*/
typedef struct _configPage2_t_ {

  S16 triggerAngle;
  U8 FixAng;
  U8 CrankAng;
  U8 TrigAngMul; //Multiplier for non evenly divisible tooth counts.

  U8 TrigEdge : 1;
  U8 TrigSpeed : 1;
  U8 IgInv : 1;
  U8 oddfire : 1;
  U8 TrigPattern : 4;

  U8 TrigEdgeSec : 1;
  U8 fuelPumpPin : 6;
  U8 useResync : 1;

  U8 sparkDur; //Spark duration in ms * 10
  U8 IdleAdvRPM;
  U8 IdleAdvCLT; //The temperature below which the idle is advanced
  U8 IdleDelayTime;
  U8 StgCycles; //The number of initial cycles before the ignition should fire when first cranking

  U8 dwellCont : 1; //Fixed duty dwell control
  U8 useDwellLim : 1; //Whether the dwell limiter is off or on
  U8 sparkMode : 3; //Spark output mode (Eg Wasted spark, single channel or Wasted COP)
  U8 triggerFilter : 2; //The mode of trigger filter being used (0=Off, 1=Light (Not currently used), 2=Normal, 3=Aggressive)
  U8 ignCranklock : 1; //Whether or not the ignition timing during cranking is locked to a CAS pulse. Only currently valid for Basic distributor and 4G63.

  U8 dwellCrank; //Dwell time whilst cranking
  U8 dwellRun; //Dwell time whilst running
  U8 triggerTeeth; //The full count of teeth on the trigger wheel if there were no gaps
  U8 triggerMissingTeeth; //The size of the tooth gap (ie number of missing teeth)
  U8 crankRPM; //RPM below which the engine is considered to be cranking
  U8 floodClear; //TPS value that triggers flood clear mode (No fuel whilst cranking)
  U8 SoftRevLim; //Soft rev limit (RPM/100)
  U8 SoftLimRetard; //Amount soft limit retards (degrees)
  U8 SoftLimMax; //Time the soft limit can run
  U8 HardRevLim; //Hard rev limit (RPM/100)
  U8 taeBins[4]; //TPS based acceleration enrichment bins (%/s)
  U8 taeValues[4]; //TPS based acceleration enrichment rates (% to add)
  U8 wueBins[10]; //Warmup Enrichment bins (Values are in configTable1)
  U8 dwellLimit;
  U8 dwellCorrectionValues[6]; //Correction table for dwell vs battery voltage
  U8 iatRetBins[6]; // Inlet Air Temp timing retard curve bins
  U8 iatRetValues[6]; // Inlet Air Temp timing retard curve values
  U8 dfcoRPM; //RPM at which DFCO turns off/on at
  U8 dfcoHyster; //Hysteris RPM for DFCO
  U8 dfcoTPSThresh; //TPS must be below this figure for DFCO to engage

  U8 ignBypassEnabled : 1; //Whether or not the ignition bypass is enabled
  U8 ignBypassPin : 6; //Pin the ignition bypass is activated on
  U8 ignBypassHiLo : 1; //Whether this should be active high or low.

} configPage2_t ;


/**
Page 3 of the config - See the ini file for further reference
This mostly covers off variables that are required for AFR targets and closed loop
*/
typedef struct _configPage3_t_ {

  U8 egoAlgorithm : 2;
  U8 egoType : 2;
  U8 boostEnabled : 1;
  U8 vvtEnabled : 1;
  U8 boostCutType : 2;

  U8 egoKP;
  U8 egoKI;
  U8 egoKD;
  U8 egoTemp; //The temperature above which closed loop functions
  U8 egoCount; //The number of ignition cylces per step
  U8 egoDelta; //The step size (In %) when using simple algorithm
  U8 egoLimit; //Maximum amount the closed loop will vary the fueling
  U8 ego_min; //AFR must be above this for closed loop to function
  U8 ego_max; //AFR must be below this for closed loop to function
  U8 ego_sdelay; //Time in seconds after engine starts that closed loop becomes available
  U8 egoRPM; //RPM must be above this for closed loop to function
  U8 egoTPSMax; //TPS must be below this for closed loop to function
  U8 vvtPin : 6;
  U8 useExtBaro : 1;
  U8 boostMode : 1; //Simple of full boost contrl
  U8 boostPin : 6;
  U8 unused6_14 : 2;
  U8 voltageCorrectionBins[6]; //X axis bins for voltage correction tables
  U8 injVoltageCorrectionValues[6]; //Correction table for injector PW vs battery voltage
  U8 airDenBins[9];
  U8 airDenRates[9];
  U8 boostFreq; //Frequency of the boost PWM valve
  U8 vvtFreq; //Frequency of the vvt PWM valve
  U8 idleFreq;

  U8 launchPin : 6;
  U8 launchEnabled : 1;
  U8 launchHiLo : 1;

  U8 lnchSoftLim;
  S8 lnchRetard; //Allow for negative advance value (ATDC)
  U8 lnchHardLim;
  U8 lnchFuelAdd;

  //PID values for idle needed to go here as out of room in the idle page
  U8 idleKP;
  U8 idleKI;
  U8 idleKD;

  U8 boostLimit; //Is divided by 2, allowing kPa values up to 511
  U8 boostKP;
  U8 boostKI;
  U8 boostKD;

  U8 lnchPullRes : 2;
  U8 fuelTrimEnabled : 1;
  U8 flatSEnable : 1;
  U8 baroPin : 4;
  U8 flatSSoftWin;
  U8 flatSRetard;
  U8 flatSArm;

} configPage3_t ;

/**
Page 4 of the config mostly deals with idle control
See ini file for further info (Config Page 7 in the ini)
*/
typedef struct _configPage4_t_ {

  U8 iacCLValues[10]; //Closed loop target RPM value
  U8 iacOLStepVal[10]; //Open loop step values for stepper motors
  U8 iacOLPWMVal[10]; //Open loop duty values for PMWM valves
  U8 iacBins[10]; //Temperature Bins for the above 3 curves
  U8 iacCrankSteps[4]; //Steps to use when cranking (Stepper motor)
  U8 iacCrankDuty[4]; //Duty cycle to use on PWM valves when cranking
  U8 iacCrankBins[4]; //Temperature Bins for the above 2 curves

  U8 iacAlgorithm : 3; //Valid values are: "None", "On/Off", "PWM", "PWM Closed Loop", "Stepper", "Stepper Closed Loop"
  U8 iacStepTime : 3; //How long to pulse the stepper for to ensure the step completes (ms)
  U8 iacChannels : 1; //How many outputs to use in PWM mode (0 = 1 channel, 1 = 2 channels)
  U8 iacPWMdir : 1; //Directino of the PWM valve. 0 = Normal = Higher RPM with more duty. 1 = Reverse = Lower RPM with more duty

  U8 iacFastTemp; //Fast idle temp when using a simple on/off valve

  U8 iacStepHome; //When using a stepper motor, the number of steps to be taken on startup to home the motor
  U8 iacStepHyster; //Hysteresis temperature (*10). Eg 2.2C = 22

  U8 fanInv : 1;        // Fan output inversion bit
  U8 fanEnable : 1;     // Fan enable bit. 0=Off, 1=On/Off
  U8 fanPin : 5;
  U8 fanSP;             // Cooling fan start temperature
  U8 fanHyster;         // Fan hysteresis
  U8 fanFreq;           // Fan PWM frequency
  U8 fanPWMBins[4];     //Temperature Bins for the PWM fan control

} configPage4_t ;


/**
Page 10 of the config mostly deals with CANBUS control
See ini file for further info (Config Page 10 in the ini)
*/
typedef struct _configPage10_t_ {

  U8 enable_canbus:2;
  U8 enable_candata_in:1;
  U16 caninput_sel;                    //bit status on/off if input is enabled
  U16 caninput_param_group[16];        //u16 [15] array holding can address of input
  U8 caninput_param_start_U8[16];     //u08 [15] array holds the start byte number(value of 0-7)
  U16 caninput_param_num_bytes;     //u16 bit status of the number of bytes length 1 or 2
  U8 unused10_53;
  U8 unused10_54;
  U8 enable_candata_out : 1;
  U8 canoutput_sel[8];
  U16 canoutput_param_group[8];
  U8 canoutput_param_start_byte[8];
  U8 canoutput_param_num_bytes[8];

  U8 unused10_97;
  U8 unused10_98;
  U8 unused10_99;
  U8 speeduino_tsCanId:4;         //speeduino TS canid (0-14)
  U16 true_address;            //speeduino 11bit can address
  U16 realtime_base_address;   //speeduino 11 bit realtime base address
  U16 obd_address;             //speeduino OBD diagnostic address
  U8 unused10_107;
  U8 unused10_108;
  U8 unused10_109;
  U8 unused10_110;
  U8 unused10_111;
  U8 unused10_112;
  U8 unused10_113;
  U8 unused10_114;
  U8 unused10_115;
  U8 unused10_116;
  U8 unused10_117;
  U8 unused10_118;
  U8 unused10_119;
  U8 unused10_120;
  U8 unused10_121;
  U8 unused10_122;
  U8 unused10_123;
  U8 unused10_124;
  U8 unused10_125;
  U8 unused10_126;
  U8 unused10_127;

} configPage10_t ;

/**
Page 11 - No specific purpose. Created initially for the cranking enrich curve
192 bytes long
See ini file for further info (Config Page 11 in the ini)
*/
typedef struct _configPage11_t_ {

  U8 crankingEnrichBins[4];
  U8 crankingEnrichValues[4];

  U8 rotaryType : 2;
  U8 unused11_8c : 6;

  U8 rotarySplitValues[8];
  U8 rotarySplitBins[8];

  U16 boostSens;
  U8 boostIntv;
  U8 unused11_28_192[164];


} configPage11_t;


#define CALIBRATION_TABLE_DIMENSION 6

/**
analog sensor calibration page
*/
typedef struct _configPage9_t_ {

    U16 IAT_calib_data_x[CALIBRATION_TABLE_DIMENSION];
    U16 IAT_calib_data_y[CALIBRATION_TABLE_DIMENSION];

    U16 CLT_calib_data_x[CALIBRATION_TABLE_DIMENSION];
    U16 CLT_calib_data_y[CALIBRATION_TABLE_DIMENSION];

    U16 TPS_calib_data_x[CALIBRATION_TABLE_DIMENSION];
    U16 TPS_calib_data_y[CALIBRATION_TABLE_DIMENSION];

    U16 MAP_calib_M;
    U16 MAP_calib_N;
    U16 MAP_calib_L;

    U16 BARO_calib_M;
    U16 BARO_calib_N;
    U16 BARO_calib_L;

    U16 O2_calib_M;
    U16 O2_calib_N;
    U16 O2_calib_L;

    U16 VBAT_calib_M;
    U16 VBAT_calib_L;

} configPage9_t ;


#endif // CONFIG_PAGES_H_INCLUDED
