

#include "utils.h"
#include "table.h"
#include "storage.h"
#include "eeprom.h"
#include "eeprom_layout.h"
#include "config_pages.h"
#include "config.h"
#include "eeprom_layout.h"

//DEBUG
#include "uart.h"
#include "conversion.h"

/**
these are our config pages
*/
volatile configPage1_t configPage1;
volatile configPage2_t configPage2;
volatile configPage3_t configPage3;
volatile configPage4_t configPage4;
volatile configPage9_t configPage9;
volatile configPage10_t configPage10;
volatile configPage11_t configPage11;



/****************************************************************************************************************************************************
*
* Load configuration data from EEPROM
*
****************************************************************************************************************************************************/
U32 load_ConfigData()
{
    U32 data, offset, x, y, z, i;
    U8 * pnt_configPage;
    U8 eeprom_data;
    U32 eeprom_code;


    /*************
    * Fuel table *
    *************/

    for(x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
    {
        offset = x - EEPROM_CONFIG1_MAP;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            fuelTable.axisZ[offset / TABLE_3D_ARRAYSIZE][offset % TABLE_3D_ARRAYSIZE]= eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }

    for(x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
    {
        offset = x - EEPROM_CONFIG1_XBINS;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            fuelTable.axisX[offset]= eeprom_data * TABLE_RPM_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }
    }

    for(x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
    {
        offset = x - EEPROM_CONFIG1_YBINS;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            fuelTable.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }
    }

    pnt_configPage = (U8 *)&configPage1;

    for(x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
    {
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            *(pnt_configPage + (U8) (x - EEPROM_CONFIG2_START)) = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }

    /***************************
    * IGNITION CONFIG PAGE (2) *
    ***************************/

    for(x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
    {
        offset = x - EEPROM_CONFIG3_MAP;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            ignitionTable.axisZ[offset / TABLE_3D_ARRAYSIZE][offset % TABLE_3D_ARRAYSIZE] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }


    for(x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
    {
        offset = x - EEPROM_CONFIG3_XBINS;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            ignitionTable.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }
    }


    for(x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
    {
        offset = x - EEPROM_CONFIG3_YBINS;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Table load is divided by 2 (Allows for MAP up to 511)
            ignitionTable.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }
    }


    pnt_configPage = (U8 *)&configPage2;

    for(x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
    {
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            *(pnt_configPage + (U8) (x - EEPROM_CONFIG4_START)) = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }

    /*****************************
    * AFR TARGET CONFIG PAGE (3) *
    *****************************/

    for(x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
    {
        offset = x - EEPROM_CONFIG5_MAP;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            afrTable.axisZ[offset / TABLE_3D_ARRAYSIZE][offset % TABLE_3D_ARRAYSIZE] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }


    for(x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
    {
        offset = x - EEPROM_CONFIG5_XBINS;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            afrTable.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }
    }


    for(x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
    {
        offset = x - EEPROM_CONFIG5_YBINS;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Table load is divided by 2 (Allows for MAP up to 511)
            afrTable.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }
    }

    pnt_configPage = (U8 *)&configPage3;

    for(x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
    {
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            *(pnt_configPage + (U8) (x - EEPROM_CONFIG6_START)) = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }

    /******************
    * CONFIG PAGE (4) *
    ******************/

    pnt_configPage = (U8 *)&configPage4;

    //The first 64 bytes can simply be pulled straight from the configTable
    for(x=EEPROM_CONFIG7_START; x<EEPROM_CONFIG7_END; x++)
    {
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            *(pnt_configPage + (U8) (x - EEPROM_CONFIG7_START)) = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }

    /****************************
    * Boost and vvt tables load *
    ****************************/

    y = EEPROM_CONFIG8_MAP2;

    for(x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
    {
        offset = x - EEPROM_CONFIG8_MAP1;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Read the 8x8 map
            boostTable.axisZ[(offset/8)][offset%8] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG8_MAP2;
        eeprom_code= eeprom_read_byte(y, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Read the 8x8 map
            vvtTable.axisZ[(offset/8)][offset%8] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        y++;
    }

    //RPM bins
    y = EEPROM_CONFIG8_XBINS2;

    for(x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
    {
        offset = x - EEPROM_CONFIG8_XBINS1;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            boostTable.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG8_XBINS2;
        eeprom_code= eeprom_read_byte(y, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            vvtTable.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }

        y++;
    }

    //TPS/MAP bins
    y = EEPROM_CONFIG8_YBINS2;

    for(x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
    {
        offset = x - EEPROM_CONFIG8_YBINS1;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
            boostTable.axisY[offset] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG8_YBINS2;

        eeprom_code= eeprom_read_byte(y, &eeprom_data);

        if(eeprom_code == 0)
        {
            //TABLE_LOAD_MULTIPLIER is NOT used for VVT as it is TPS based (0-100)
            vvtTable.axisY[offset] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        y++;
    }

    /************************
    * Fuel trim tables load *
    ************************/

    y = EEPROM_CONFIG9_MAP2;
    z = EEPROM_CONFIG9_MAP3;
    i = EEPROM_CONFIG9_MAP4;

    for(x=EEPROM_CONFIG9_MAP1; x<EEPROM_CONFIG9_XBINS1; x++)
    {
        offset = x - EEPROM_CONFIG9_MAP1;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            trim1Table.axisZ[(offset/6)][offset%6] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG9_MAP2;
        eeprom_code= eeprom_read_byte(y, &eeprom_data);

        if(eeprom_code == 0)
        {
            trim2Table.axisZ[(offset/6)][offset%6] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        offset = z - EEPROM_CONFIG9_MAP3;
        eeprom_code= eeprom_read_byte(z, &eeprom_data);

        if(eeprom_code == 0)
        {
            trim3Table.axisZ[(offset/6)][offset%6] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        offset = i - EEPROM_CONFIG9_MAP4;
        eeprom_code= eeprom_read_byte(i, &eeprom_data);

        if(eeprom_code == 0)
        {
            trim4Table.axisZ[(offset/6)][offset%6] = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }

        y++;
        z++;
        i++;
    }

    //RPM bins
    y = EEPROM_CONFIG9_XBINS2;
    z = EEPROM_CONFIG9_XBINS3;
    i = EEPROM_CONFIG9_XBINS4;

    for(x=EEPROM_CONFIG9_XBINS1; x<EEPROM_CONFIG9_YBINS1; x++)
    {
        offset = x - EEPROM_CONFIG9_XBINS1;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            trim1Table.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }


        offset = y - EEPROM_CONFIG9_XBINS2;
        eeprom_code= eeprom_read_byte(y, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            trim2Table.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);

        }
        else
        {
            return eeprom_code;
        }

        offset = z - EEPROM_CONFIG9_XBINS3;
        eeprom_code= eeprom_read_byte(z, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            trim3Table.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }


        offset = i - EEPROM_CONFIG9_XBINS4;
        eeprom_code= eeprom_read_byte(i, &eeprom_data);

        if(eeprom_code == 0)
        {
            //RPM bins are divided by 100 when stored. Multiply them back now
            trim4Table.axisX[offset] = (eeprom_data * TABLE_RPM_MULTIPLIER);
        }
        else
        {
            return eeprom_code;
        }


        y++;
        z++;
        i++;

    }

    //TPS/MAP bins
    y = EEPROM_CONFIG9_YBINS2;
    z = EEPROM_CONFIG9_YBINS3;
    i = EEPROM_CONFIG9_YBINS4;

    for(x=EEPROM_CONFIG9_YBINS1; x<EEPROM_CONFIG9_XSIZE2; x++)
    {
        offset = x - EEPROM_CONFIG9_YBINS1;
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Table load is divided by 2 (Allows for MAP up to 511)
            trim1Table.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }


        offset = y - EEPROM_CONFIG9_YBINS2;
        eeprom_code= eeprom_read_byte(y, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Table load is divided by 2 (Allows for MAP up to 511)
            trim2Table.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }


        offset = z - EEPROM_CONFIG9_YBINS3;
        eeprom_code= eeprom_read_byte(z, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Table load is divided by 2 (Allows for MAP up to 511)
            trim3Table.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }


        offset = i - EEPROM_CONFIG9_YBINS4;
        eeprom_code= eeprom_read_byte(i, &eeprom_data);

        if(eeprom_code == 0)
        {
            //Table load is divided by 2 (Allows for MAP up to 511)
            trim4Table.axisY[offset] = eeprom_data * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
            return eeprom_code;
        }

        y++;
        z++;
        i++;
    }

    /*******************
    * CONFIG PAGE (11) *
    *******************/

    pnt_configPage = (U8 *)&configPage11;

    //All 192 bytes can simply be pulled straight from the configTable
    for(x=EEPROM_CONFIG11_START; x<EEPROM_CONFIG11_END; x++)
    {
        eeprom_code= eeprom_read_byte(x, &eeprom_data);

        if(eeprom_code == 0)
        {
            *(pnt_configPage + (U8) (x - EEPROM_CONFIG11_START)) = eeprom_data;
        }
        else
        {
            return eeprom_code;
        }
    }

    /*******************
    * CONFIG PAGE 9    *
    * (CALIBRATION)    *
    *******************/

    //all calibration tables have the same size
    for(i=0; i < CALIBRATION_TABLE_DIMENSION; i++)
    {
        /**
        CLT
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_CLT_X + i* EEPROM_CALIB_DATA_WIDTH, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.CLT_calib_data_x[i]= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_CLT_Y + i* EEPROM_CALIB_DATA_WIDTH, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.CLT_calib_data_y[i]= (U16) data;


        /**
        IAT
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_IAT_X + i* EEPROM_CALIB_DATA_WIDTH, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.IAT_calib_data_x[i]= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_IAT_Y + i* EEPROM_CALIB_DATA_WIDTH, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.IAT_calib_data_y[i]= (U16) data;


        /**
        TPS
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_TPS_X + i* EEPROM_CALIB_DATA_WIDTH, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.TPS_calib_data_x[i]= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_TPS_Y + i* EEPROM_CALIB_DATA_WIDTH, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.TPS_calib_data_y[i]= (U16) data;


        /**
        MAP
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_MAP_M, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.MAP_calib_M= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_MAP_N, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.MAP_calib_N= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_MAP_L, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.MAP_calib_L= (U16) data;


        /**
        BARO
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_BARO_M, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.BARO_calib_M= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_BARO_N, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.BARO_calib_N= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_BARO_L, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.BARO_calib_L= (U16) data;


        /**
        O2 sensor
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_O2_M, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.O2_calib_M= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_O2_N, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.O2_calib_N= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_O2_L, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.O2_calib_L= (U16) data;


        /**
        battery voltage sensor
        */
        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_VBAT_M, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.VBAT_calib_M= (U16) data;


        eeprom_code= eeprom_read_bytes(EEPROM_CALIBRATION_VBAT_L, &data, 2);

        //exit here if eeprom read failed
        if(eeprom_code)
        {
            return eeprom_code;
        }

        configPage9.VBAT_calib_L= (U16) data;

    }


    //success
    return 0;

}



/****************************************************************************************************************************************************
*
* Takes the current configuration (config pages and maps)
* and writes them to EEPROM as per the layout defined in eeprom_layout.h
*
TODO remove map dimensions from eeprom
****************************************************************************************************************************************************/
U32 write_ConfigData()
{

    U16 offset, x, y, z, i;
    U8 * pnt_configPage;
    U32 eeprom_code;


    /*************
    * Fuel table *
    *************/

    eeprom_code= eeprom_update(EEPROM_CONFIG1_XSIZE, fuelTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    for(x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
    {
        offset = x - EEPROM_CONFIG1_MAP;

        eeprom_code= eeprom_update(x, (fuelTable.axisZ[offset / TABLE_3D_ARRAYSIZE][offset % TABLE_3D_ARRAYSIZE]));

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    //RPM bins
    for(x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
    {
        offset = x - EEPROM_CONFIG1_XBINS;

        eeprom_code= eeprom_update(x, (fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    //TPS/MAP bins
    for(x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
    {
        offset = x - EEPROM_CONFIG1_YBINS;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code= eeprom_update(x, fuelTable.axisY[offset] / TABLE_LOAD_MULTIPLIER);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }


    /*******************
    * CONFIG PAGE (1)  *
    *******************/

    pnt_configPage = (U8 *)&configPage1;

    for(x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
    {
        eeprom_code= eeprom_update(x, *(pnt_configPage + (U8)(x - EEPROM_CONFIG2_START)) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }


    /*****************
    * Ignition table *
    *****************/

    eeprom_code= eeprom_update(EEPROM_CONFIG3_XSIZE, ignitionTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    for(x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
    {
        offset = x - EEPROM_CONFIG3_MAP;

        eeprom_code= eeprom_update(x, (ignitionTable.axisZ[offset / TABLE_3D_ARRAYSIZE][offset % TABLE_3D_ARRAYSIZE]) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    //RPM bins
    for( x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
    {
        offset = x - EEPROM_CONFIG3_XBINS;

        eeprom_code= eeprom_update(x, (U8)(ignitionTable.axisX[offset]/TABLE_RPM_MULTIPLIER) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    //TPS/MAP bins
    for( x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
    {
        offset = x - EEPROM_CONFIG3_YBINS;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code= eeprom_update(x, ignitionTable.axisY[offset]/TABLE_LOAD_MULTIPLIER);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }



    /*******************
    * CONFIG PAGE (2)  *
    *******************/

    pnt_configPage = (U8 *)&configPage2;

    for(x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
    {
        eeprom_code= eeprom_update(x, *(pnt_configPage + (U8)(x - EEPROM_CONFIG4_START)) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }


    /*************
    * AFR TABLE  *
    *************/

    eeprom_code= eeprom_update(EEPROM_CONFIG5_XSIZE, afrTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    eeprom_code= eeprom_update(EEPROM_CONFIG5_YSIZE, afrTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }


    for( x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
    {
        offset = x - EEPROM_CONFIG5_MAP;

        eeprom_code= eeprom_update(x, (afrTable.axisZ[offset / TABLE_3D_ARRAYSIZE][offset % TABLE_3D_ARRAYSIZE]) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    //RPM bins
    for(x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
    {
        offset = x - EEPROM_CONFIG5_XBINS;

        eeprom_code= eeprom_update(x, (U8) (afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    //TPS/MAP bins
    for(x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
    {
        offset = x - EEPROM_CONFIG5_YBINS;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code= eeprom_update(x, afrTable.axisY[offset]/TABLE_LOAD_MULTIPLIER);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }


    /*******************
    * CONFIG PAGE (2)  *
    *******************/

    pnt_configPage = (U8 *)&configPage3;

    for(x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
    {
        eeprom_code= eeprom_update(x, *(pnt_configPage + (U8) (x - EEPROM_CONFIG6_START)) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    /*******************
    * CONFIG PAGE (4)  *
    *******************/

    pnt_configPage = (U8 *)&configPage4;

    for(x=EEPROM_CONFIG7_START; x<EEPROM_CONFIG7_END; x++)
    {
        eeprom_code= eeprom_update(x, *(pnt_configPage + (U8) (x - EEPROM_CONFIG7_START)) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }

    /****************************
    * Boost and vvt tables load *
    ****************************/

    eeprom_code= eeprom_update(EEPROM_CONFIG8_XSIZE1, boostTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    eeprom_code= eeprom_update(EEPROM_CONFIG8_YSIZE1, boostTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    eeprom_code= eeprom_update(EEPROM_CONFIG8_XSIZE2, vvtTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    eeprom_code= eeprom_update(EEPROM_CONFIG8_YSIZE2, vvtTable.dimension);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }


    y = EEPROM_CONFIG8_MAP2;

    for(x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
    {
        offset = x - EEPROM_CONFIG8_MAP1;

        eeprom_code= eeprom_update(x, (boostTable.axisZ[(offset/8)][offset%8]) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG8_MAP2;

        eeprom_code= eeprom_update(y, (vvtTable.axisZ[(offset/8)][offset%8]) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        y++;
    }

    //RPM bins
    y = EEPROM_CONFIG8_XBINS2;

    for(x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
    {
        offset = x - EEPROM_CONFIG8_XBINS1;

        eeprom_code= eeprom_update(x, (U8) (boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG8_XBINS2;

        //RPM bins are divided by 100
        eeprom_code= eeprom_update(y, (U8) (vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        y++;
    }

    //TPS/MAP bins
    y=EEPROM_CONFIG8_YBINS2;

    for(x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
    {
        offset = x - EEPROM_CONFIG8_YBINS1;

        //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
        eeprom_code= eeprom_update(x, boostTable.axisY[offset]);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG8_YBINS2;

        //TABLE_LOAD_MULTIPLIER is NOT used for VVT as it is TPS based (0-100)
        eeprom_code= eeprom_update(y, vvtTable.axisY[offset]);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        y++;
  }

    /*******************
    * Fuel trim tables *
    *******************/

    //Write the boost Table RPM dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_XSIZE1,trim1Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table MAP/TPS dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_YSIZE1,trim1Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table RPM dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_XSIZE2,trim2Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table MAP/TPS dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_YSIZE2,trim2Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table RPM dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_XSIZE3,trim3Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table MAP/TPS dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_YSIZE3,trim3Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table RPM dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_XSIZE4,trim4Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    //Write the boost Table MAP/TPS dimension size
    eeprom_code= eeprom_update(EEPROM_CONFIG9_YSIZE4,trim4Table.dimension);

    if(eeprom_code != 0)
        {
            return eeprom_code;
        }

    y = EEPROM_CONFIG9_MAP2;
    z = EEPROM_CONFIG9_MAP3;
    i = EEPROM_CONFIG9_MAP4;

    for(x=EEPROM_CONFIG9_MAP1; x<EEPROM_CONFIG9_XBINS1; x++)
    {
        offset = x - EEPROM_CONFIG9_MAP1;
        eeprom_code= eeprom_update(x, (trim1Table.axisZ[(offset/6)][offset%6]) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG9_MAP2;
        eeprom_code= eeprom_update(y, trim2Table.axisZ[(offset/6)][offset%6]);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = z - EEPROM_CONFIG9_MAP3;
        eeprom_code= eeprom_update(z, trim3Table.axisZ[(offset/6)][offset%6]);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = i - EEPROM_CONFIG9_MAP4;
        eeprom_code= eeprom_update(i, trim4Table.axisZ[(offset/6)][offset%6]);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        y++;
        z++;
        i++;
    }

    //RPM bins
    y = EEPROM_CONFIG9_XBINS2;
    z = EEPROM_CONFIG9_XBINS3;
    i = EEPROM_CONFIG9_XBINS4;

    for(x=EEPROM_CONFIG9_XBINS1; x<EEPROM_CONFIG9_YBINS1; x++)
    {
        offset = x - EEPROM_CONFIG9_XBINS1;

        //RPM bins are divided by 100 and converted to a byte
        eeprom_code= eeprom_update(x, (U8) (trim1Table.axisX[offset]/TABLE_RPM_MULTIPLIER));

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG9_XBINS2;

        //RPM bins are divided by 100 and converted to a byte
        eeprom_code= eeprom_update(y, (U8) (trim2Table.axisX[offset]/TABLE_RPM_MULTIPLIER));

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = z - EEPROM_CONFIG9_XBINS3;

        //RPM bins are divided by 100 and converted to a byte
        eeprom_code= eeprom_update(z, (U8) (trim3Table.axisX[offset]/TABLE_RPM_MULTIPLIER));

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = i - EEPROM_CONFIG9_XBINS4;

        //RPM bins are divided by 100 and converted to a byte
        eeprom_code= eeprom_update(i, (U8) (trim4Table.axisX[offset]/TABLE_RPM_MULTIPLIER));

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        y++;
        z++;
        i++;
    }

    //TPS/MAP bins
    y=EEPROM_CONFIG9_YBINS2;
    z=EEPROM_CONFIG9_YBINS3;
    i=EEPROM_CONFIG9_YBINS4;

    for(x=EEPROM_CONFIG9_YBINS1; x<EEPROM_CONFIG9_XSIZE2; x++)
    {
        offset = x - EEPROM_CONFIG9_YBINS1;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code= eeprom_update(x, trim1Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = y - EEPROM_CONFIG9_YBINS2;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code=  eeprom_update(y, trim2Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);

       if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = z - EEPROM_CONFIG9_YBINS3;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code= eeprom_update(z, trim3Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        offset = i - EEPROM_CONFIG9_YBINS4;

        //Table load is divided by 2 (Allows for MAP up to 511)
        eeprom_code= eeprom_update(i, trim4Table.axisY[offset]/TABLE_LOAD_MULTIPLIER);

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }

        y++;
        z++;
        i++;
    }


    /********************
    * CONFIG PAGE (10)  *
    ********************/

    pnt_configPage = (U8 *)&configPage10;

    for(x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
    {
        eeprom_code= eeprom_update(x, *(pnt_configPage + (U8) (x - EEPROM_CONFIG10_START)) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }


    /********************
    * CONFIG PAGE (11)  *
    ********************/

    pnt_configPage = (U8 *)&configPage11;

    for(x=EEPROM_CONFIG11_START; x<EEPROM_CONFIG11_END; x++)
    {
        eeprom_code= eeprom_update(x, *(pnt_configPage + (U8) (x - EEPROM_CONFIG11_START)) );

        if(eeprom_code != 0)
        {
            return eeprom_code;
        }
    }


    /*******************
    * CONFIG PAGE 9    *
    * (CALIBRATION)    *
    *******************/

    //all calibration tables have the same size
    for(i=0; i < CALIBRATION_TABLE_DIMENSION; i++)
    {
        /**
        IAT
        */
        eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_IAT_X + i* EEPROM_CALIB_DATA_WIDTH, configPage9.IAT_calib_data_x[i], 2);
        eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_IAT_Y + i* EEPROM_CALIB_DATA_WIDTH, configPage9.IAT_calib_data_y[i], 2);

        if(eeprom_code)
        {
            return eeprom_code;
        }

        /**
        CLT
        */
        eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_CLT_X + i* EEPROM_CALIB_DATA_WIDTH, configPage9.CLT_calib_data_x[i], 2);
        eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_CLT_Y + i* EEPROM_CALIB_DATA_WIDTH, configPage9.CLT_calib_data_y[i], 2);

        if(eeprom_code)
        {
            return eeprom_code;
        }

        /**
        TPS
        */
        eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_TPS_X + i* EEPROM_CALIB_DATA_WIDTH, configPage9.TPS_calib_data_x[i], 2);
        eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_TPS_Y + i* EEPROM_CALIB_DATA_WIDTH, configPage9.TPS_calib_data_y[i], 2);

        if(eeprom_code)
        {
            return eeprom_code;
        }


    }

    /**
    MAP
    */
    eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_MAP_M, configPage9.MAP_calib_M, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_MAP_N, configPage9.MAP_calib_N, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_MAP_L, configPage9.MAP_calib_L, 2);

    if(eeprom_code)
    {
        return eeprom_code;
    }

    /**
    BARO
    */
    eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_BARO_M, configPage9.BARO_calib_M, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_BARO_N, configPage9.BARO_calib_N, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_BARO_L, configPage9.BARO_calib_L, 2);

    if(eeprom_code)
    {
        return eeprom_code;
    }

    /**
    O2
    */
    eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_O2_M, configPage9.O2_calib_M, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_O2_N, configPage9.O2_calib_N, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_O2_L, configPage9.O2_calib_L, 2);

    if(eeprom_code)
    {
        return eeprom_code;
    }

    /**
    battery voltage sensor
    */
    eeprom_code= eeprom_update_bytes(EEPROM_CALIBRATION_VBAT_M, configPage9.VBAT_calib_M, 2);
    eeprom_code += eeprom_update_bytes(EEPROM_CALIBRATION_VBAT_L, configPage9.VBAT_calib_L, 2);

    if(eeprom_code)
    {
        return eeprom_code;
    }



    return 0;
}



/****************************************************************************************************************************************************
 * This routine is used for doing any data conversions that are required during firmware changes
 * This prevents users getting difference reports in TS when such a data change occurs.
 * It also can be used for setting good values when there are variables that move locations in the ini
 * When a user skips multiple firmware versions at a time, this will roll through the updates 1 at a time
 ****************************************************************************************************************************************************/
U32 migrate_configData()
{
    U8 eeprom_data;
    U32 eeprom_code;


    /**
    check DATA_VERSION
    (Brand new eeprom)
    */
    eeprom_code= eeprom_read_byte(EEPROM_DATA_VERSION, &eeprom_data);

    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    if( (eeprom_data == 0) || (eeprom_data == 255) )
    {
        eeprom_code= eeprom_update(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION);
    }

    return eeprom_code;
}


