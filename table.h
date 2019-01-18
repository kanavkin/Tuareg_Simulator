/*
This file is used for everything related to maps/tables including their definition, functions etc
*/
#ifndef TABLE_H
#define TABLE_H
#include "types.h"

#define TABLE_RPM_MULTIPLIER  100
#define TABLE_LOAD_MULTIPLIER 2


#define FUEL_TABLE_DIMENSION 16
#define IGNITION_TABLE_DIMENSION 16
#define AFR_TABLE_DIMENSION 16
#define BOOST_TABLE_DIMENSION 8
#define VVT_TABLE_DIMENSION 8
#define TRIM1_TABLE_DIMENSION 6
#define TRIM2_TABLE_DIMENSION 6
#define TRIM3_TABLE_DIMENSION 6
#define TRIM4_TABLE_DIMENSION 6


/**
Calibration tables
use linear interpolation for sensor data conversion (we can afford the the table overhead with low conversion rates, and save a lot of RAM)
All temperature measurements are stored offset by 40 degrees. This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 215
The fuel trim tables are offset by 128 to allow for -128 to +128 values
Ignition values from the main spark table are offset 40 degrees downards to allow for negative spark timing
*/


//#define CALIBRATION_TABLE_SIZE 512

#define OFFSET_FUELTRIM 127
#define OFFSET_IGNITION 40


#define TABLE_3D_ARRAYSIZE 16


/**
This 2D table handles 16-bit unsigned values

It actually does not store any configuration data
(config data lives in the configpages),
but provide quick access to them
*/
typedef struct _table2D_t {

  //number of tupels
  U32 dimension;

  VU16 * axisX;
  VU16 * axisY;

  /**
  Store the X environment from the last
  request to make the next access faster
  */
  U32 last_Xmax_index;

  //not needed anymore, to be removed
  U32 last_Xmin_index;

} table2D ;



/**
This 3D table "contains" 8-bit unsigned values in Z,
16-bit signed values in X and Y

It actually does not store any configuration data
(config data lives in the configpages),
but provide quick access to them

Z = f(X, Y)

TODO
check if we really need S16 values in X,Y
axisX is mostly RPM
//RPM bins are divided by 100 and converted to U8 at eeprom write
*/
typedef struct _table3D_t {

    //number of tripels
    //to be removed soon
    U32 dimension;

    /**
    Store the X and Y interval from the last
    request to make the next access faster
    */
    U32 last_Xmax_index :8;
    U32 last_Xmin_index :8;
    U32 last_Ymax_index :8;
    U32 last_Ymin_index :8;

    U8 axisZ[TABLE_3D_ARRAYSIZE] [TABLE_3D_ARRAYSIZE];
    U16 axisX[TABLE_3D_ARRAYSIZE];
    U16 axisY[TABLE_3D_ARRAYSIZE];

} table3D ;


void init_3Dtables();
void init_2Dtables();
U32 table2D_getValue(volatile table2D *fromTable, U32 X);
U32 table3D_getValue(volatile table3D * fromTable, S32 X, S32 Y);


extern volatile table3D fuelTable, ignitionTable, afrTable;
extern volatile table3D boostTable, vvtTable;
extern volatile table3D trim1Table, trim2Table, trim3Table, trim4Table;

extern volatile table2D taeTable;
extern volatile table2D WUETable;
extern volatile table2D crankingEnrichTable;
extern volatile table2D dwellVCorrectionTable;
extern volatile table2D injectorVCorrectionTable;
extern volatile table2D IATDensityCorrectionTable;
extern volatile table2D IATRetardTable;
extern volatile table2D rotarySplitTable;
extern volatile table2D IAT_calib_table;
extern volatile table2D CLT_calib_table;
extern volatile table2D TPS_calib_table;




#endif // TABLE_H
