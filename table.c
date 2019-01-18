/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include <stdlib.h>
#include "types.h"

#include "table.h"
#include "config.h"

#include "uart.h"
#include "conversion.h"
#include <math.h>


/**
table structs
*/
volatile table3D fuelTable, ignitionTable, afrTable;
volatile table3D boostTable, vvtTable;
volatile table3D trim1Table, trim2Table, trim3Table, trim4Table;
volatile table2D taeTable, WUETable, crankingEnrichTable, dwellVCorrectionTable;
volatile table2D injectorVCorrectionTable, IATDensityCorrectionTable, IATRetardTable, rotarySplitTable;
volatile table2D IAT_calib_table, CLT_calib_table, TPS_calib_table;





/**
TODO
to be removed soon
as we can make sure that all array is used
*/
void init_3Dtables()
{
    fuelTable.dimension= TABLE_3D_ARRAYSIZE;
    ignitionTable.dimension= TABLE_3D_ARRAYSIZE;
    afrTable.dimension= TABLE_3D_ARRAYSIZE;
    boostTable.dimension= BOOST_TABLE_DIMENSION;
    vvtTable.dimension= VVT_TABLE_DIMENSION;
    trim1Table.dimension= TRIM1_TABLE_DIMENSION;
    trim2Table.dimension= TRIM2_TABLE_DIMENSION;
    trim3Table.dimension= TRIM3_TABLE_DIMENSION;
    trim4Table.dimension= TRIM4_TABLE_DIMENSION;
}



/**
    Repoint the 2D table structs to the config pages
    (initialise the 8 table2D structs)
*/
void init_2Dtables()
{
    IAT_calib_table.dimension= CALIBRATION_TABLE_DIMENSION;
    IAT_calib_table.axisX= configPage9.IAT_calib_data_x;
    IAT_calib_table.axisY= configPage9.IAT_calib_data_y;

    CLT_calib_table.dimension= CALIBRATION_TABLE_DIMENSION;
    CLT_calib_table.axisX= configPage9.CLT_calib_data_x;
    CLT_calib_table.axisY= configPage9.CLT_calib_data_y;

    TPS_calib_table.dimension= CALIBRATION_TABLE_DIMENSION;
    TPS_calib_table.axisX= configPage9.TPS_calib_data_x;
    TPS_calib_table.axisY= configPage9.TPS_calib_data_y;



    taeTable.dimension = 4;
    taeTable.axisY = (U16 *) configPage2.taeValues;
    taeTable.axisX = (U16 *)configPage2.taeBins;

    WUETable.dimension = 10;
    WUETable.axisY = (U16 *) configPage1.wueValues;
    WUETable.axisX = (U16 *) configPage2.wueBins;

    crankingEnrichTable.dimension = 4;
    crankingEnrichTable.axisY = (U16 *) configPage11.crankingEnrichValues;
    crankingEnrichTable.axisX = (U16 *) configPage11.crankingEnrichBins;

    dwellVCorrectionTable.dimension = 6;
    dwellVCorrectionTable.axisY = (U16 *) configPage2.dwellCorrectionValues;
    dwellVCorrectionTable.axisX =  (U16 *)configPage3.voltageCorrectionBins;

    injectorVCorrectionTable.dimension = 6;
    injectorVCorrectionTable.axisY = (U16 *) configPage3.injVoltageCorrectionValues;
    injectorVCorrectionTable.axisX = (U16 *) configPage3.voltageCorrectionBins;

    IATDensityCorrectionTable.dimension = 9;
    IATDensityCorrectionTable.axisY = (U16 *) configPage3.airDenRates;
    IATDensityCorrectionTable.axisX = (U16 *) configPage3.airDenBins;

    IATRetardTable.dimension = 6;
    IATRetardTable.axisY = (U16 *) configPage2.iatRetValues;
    IATRetardTable.axisX = (U16 *) configPage2.iatRetBins;

    rotarySplitTable.dimension = 8;
    rotarySplitTable.axisY = (U16 *) configPage11.rotarySplitValues;
    rotarySplitTable.axisX = (U16 *) configPage11.rotarySplitBins;
}



/**
This function pulls a 1D linear interpolated value from a 2D table
*/
U32 table2D_getValue(volatile table2D *fromTable, U32 X)
{
    S32 xMin, xMax, xMax_index, i;
    S32 yMin, yMax, m, y;

    /**
    clip the requested X to fit the interval covered by this table
    (borrowing xM variables)
    */
    xMin = fromTable->axisX[0];
    xMax = fromTable->axisX[ fromTable->dimension -1 ];
    if(X > xMax) { X = xMax; }
    if(X < xMin) { X = xMin; }

    /**
    check if we're still in the same X interval as last time
    */
    xMax = fromTable->axisX[fromTable->last_Xmax_index];
    xMin = fromTable->axisX[fromTable->last_Xmax_index -1];

    if ( (X < xMax) && (X > xMin) )
    {
        //xM already set
        xMax_index = fromTable->last_Xmax_index;
    }
    else
    {
        /**
        Loop from the end to find a suitable x interval
        */
        for(i = fromTable->dimension-1; i >= 0; i--)
        {
            /**
            quick exit: direct fit
            the requested X value has been found among the X values
            -> take Y from the defined values

            or

            Last exit:
            looping through the values from high to low has not revealed
            a suitable X interval
            -> take the minimum defined Y value

            */
            if ( (X == fromTable->axisX[i]) || (i == 0) )
            {
                return fromTable->axisY[i];
            }

            /**
            interval fit approach:
            as X is not a direct fit it could be
            between axisX[i] and axisX[i-1]
            */
            if ( (X < fromTable->axisX[i]) && (X > fromTable->axisX[i-1]) )
            {
                //found!
                xMax_index= i;

                //store for next time
                fromTable->last_Xmax_index= xMax_index;
                break;
            }
        }
    }

    /**
    xMin/Max indexes found, look up data for calculation
    */
    xMax= fromTable->axisX[xMax_index];
    xMin= fromTable->axisX[xMax_index -1];
    yMax= fromTable->axisY[xMax_index];
    yMin= fromTable->axisY[xMax_index -1];

    /**
    y= ( k_m * (dY/dX) * (X - xMin) + k_m * yMin ) / k_m
    scaling factor k := 10000
    */
    m= (10000 * (yMax - yMin)) / (xMax - xMin);
    y= m * (X - xMin) + (yMin * 10000);

    return (y / 10000);
}



/**
This function pulls a value from a 3D table given a target for X and Y coordinates.
It performs a bilinear interpolation
*/
U32 table3D_getValue(volatile table3D * fromTable, S32 X, S32 Y)
{
    S32 A, B, C, D;
    S32 xMin, xMax, yMin, yMax;
    S32 xMin_index, xMax_index, yMin_index, yMax_index;
    S32 i;

    /**
    X handling
    */

    /**
    clip the requested X to fit the interval covered by this table
    (borrowing xM variables)
    */
    xMin = fromTable->axisX[0];
    xMax = fromTable->axisX[ fromTable->dimension -1 ];
    if(X > xMax) { X = xMax; }
    if(X < xMin) { X = xMin; }

    /**
    check if we're still in the same X interval as last time
    preset xM with the ones from last request
    */
    xMax = fromTable->axisX[fromTable->last_Xmax_index];
    xMin = fromTable->axisX[fromTable->last_Xmax_index -1];

    /**
    check if we're still in the same X environment as last time
    or if it is in a neighbor cell
    (engine rpm changes slowly between cycles)
    */
    if( (X < xMax) && (X > xMin) )
    {
        //xM already set
        xMax_index = fromTable->last_Xmax_index;
        xMin_index = xMax_index -1;
        //last_xMax_index remains valid

    }
    else if ( ((fromTable->last_Xmax_index + 1) < fromTable->dimension ) && (X > xMax) && (X < fromTable->axisX[fromTable->last_Xmax_index +1 ])  )
    {
        //x is in right neighbor interval
        xMax_index= fromTable->last_Xmax_index + 1;
        xMin_index= fromTable->last_Xmax_index;
        xMax= fromTable->axisX[xMax_index];
        xMin= fromTable->axisX[xMin_index];

        //store for next time
        fromTable->last_Xmax_index= xMax_index;
    }
    else if ( (fromTable->last_Xmax_index > 1 ) && (X < xMin) && (X > fromTable->axisX[fromTable->last_Xmax_index -2]) )
    {
        //x is in left neighbor interval
        xMax_index= fromTable->last_Xmax_index -1;
        xMin_index= fromTable->last_Xmin_index -2;
        xMax= fromTable->axisX[xMax_index];
        xMin= fromTable->axisX[xMin_index];

        //store for next time
        fromTable->last_Xmax_index= xMax_index;
    }
    else
    {
        /**
        Loop from the end to find a suitable x interval
        */
        for(i = fromTable->dimension-1; i >= 0; i--)
        {
            /**
            If the requested X value has been directly found on the x axis
            but we have to provide a suitable x interval for interpolation
            */
            if ( (X == fromTable->axisX[i]) || (i == 0) )
            {
                if(i == 0)
                {
                    /**
                    first element  can be xMin only
                    interpolation square to the right
                    */
                    xMax_index= i+1;
                    xMin_index= i;
                }
                else
                {
                    /**
                    take i as xMax
                    interpolation square to the left
                    */
                    xMax_index= i;
                    xMin_index= i-1;
                }

                xMax= fromTable->axisX[xMax_index];
                xMin= fromTable->axisX[xMin_index];
                break;
            }

            /**
            Standard scenario
            The requested X value is between axisX[i] and axisX[i-1]
            */
            if ( (X < fromTable->axisX[i]) && (X > fromTable->axisX[i-1]) )
            {
                xMax= fromTable->axisX[i];
                xMin= fromTable->axisX[i-1];
                xMax_index= i;
                xMin_index= i-1;
                fromTable->last_Xmax_index= xMax_index;
                break;
            }
        }
    }

    /**
    Y handling
    */

    /**
    clip the requested Y to fit the interval covered by this table
    (borrowing yM variables)
    */
    yMin = fromTable->axisY[0];
    yMax = fromTable->axisY[ fromTable->dimension -1 ];
    if(Y > yMax) { Y = yMax; }
    if(Y < yMin) { Y = yMin; }

    /**
    check if we're still in the same Y interval as last time
    preset xM with the ones from last request
    */
    yMax = fromTable->axisY[fromTable->last_Ymax_index];
    yMin = fromTable->axisY[fromTable->last_Ymax_index -1];

    /**
    check if we're still in the same Y environment as last time
    or if it is in a neighbor cell
    (engine rpm changes slowly between cycles)
    */
    if( (Y < yMax) && (Y > yMin) )
    {
        //yM already set
        yMax_index = fromTable->last_Ymax_index;
        yMin_index = yMax_index -1;
        //last_yMax_index remains valid

    }
    else if ( ((fromTable->last_Ymax_index + 1) < fromTable->dimension ) && (Y > yMax) && (Y < fromTable->axisY[fromTable->last_Ymax_index +1 ])  )
    {
        //y is in right neighbor interval
        yMax_index= fromTable->last_Ymax_index + 1;
        yMin_index= fromTable->last_Ymax_index;
        yMax= fromTable->axisY[yMax_index];
        yMin= fromTable->axisY[yMin_index];

        //store for next time
        fromTable->last_Ymax_index= yMax_index;
    }
    else if ( (fromTable->last_Ymax_index > 1 ) && (Y < yMin) && (Y > fromTable->axisY[fromTable->last_Ymax_index -2]) )
    {
        //y is in left neighbor interval
        yMax_index= fromTable->last_Ymax_index -1;
        yMin_index= fromTable->last_Ymin_index -2;
        yMax= fromTable->axisY[yMax_index];
        yMin= fromTable->axisY[yMin_index];

        //store for next time
        fromTable->last_Ymax_index= yMax_index;
    }
    else
    {
        /**
        Loop from the end to find a suitable y interval
        */
        for(i = fromTable->dimension-1; i >= 0; i--)
        {
            /**
            If the requested Y value has been directly found on the y axis
            but we have to provide a suitable y interval for interpolation
            */
            if ( (Y == fromTable->axisY[i]) || (i == 0) )
            {
                if(i == 0)
                {
                    /**
                    first element  can be yMin only
                    interpolation square to the right
                    */
                    yMax_index= i+1;
                    yMin_index= i;
                }
                else
                {
                    /**
                    take i as yMax
                    interpolation square to the left
                    */
                    yMax_index= i;
                    yMin_index= i-1;
                }

                yMax= fromTable->axisY[yMax_index];
                yMin= fromTable->axisY[yMin_index];
                break;
            }

            /**
            Standard scenario
            The requested Y value is between axisY[i] and axisY[i-1]
            */
            if ( (Y < fromTable->axisY[i]) && (Y > fromTable->axisY[i-1]) )
            {
                yMax= fromTable->axisY[i];
                yMin= fromTable->axisY[i-1];
                yMax_index= i;
                yMin_index= i-1;
                fromTable->last_Ymax_index= yMax_index;
                break;
            }
        }
    }

    /*************************************************
    At this point we have the 4 corners of the map
    where the interpolated value will fall in

    C(yMax,xMin)  D(yMax,xMax)
    A(yMin,xMin)  B(yMin,xMax)
    ************************************************/
    A= fromTable->axisZ[yMin_index][xMin_index];
    B= fromTable->axisZ[yMin_index][xMax_index];
    C= fromTable->axisZ[yMax_index][xMin_index];
    D= fromTable->axisZ[yMax_index][xMax_index];

    /**
    Check that all values aren't just the same
    (This regularly happens with things like the fuel trim maps)
    */
    if( (A == B) && (A == C) && (A == D) )
    {
        return A;
    }

    /**
    RESULTS finally

    */
    A *= (xMax - X) * (yMax - Y);
    B *= (X -xMin)  * (yMax - Y);
    C *= (xMax - X) * (Y - yMin);
    D *= (X - xMin) * (Y -yMin);

    return (A + B + C +D) / ((xMax - xMin) * (yMax - yMin));
}
