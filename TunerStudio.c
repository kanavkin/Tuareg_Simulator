/**
TODO
- implement diag buttons
    (ts_commandButtons)
*/




#include "utils.h"
#include "table.h"

#include "uart.h"
#include "conversion.h"
#include "TunerStudio.h"

#include "config_pages.h"
#include "config.h"
#include "Tuareg.h"
#include "eeprom.h"
#include "eeprom_layout.h"
#include "sensors.h"


volatile tuners_cli_t TS_cli;

/**
This is called when a command is received over serial from TunerStudio / Megatune
It parses the command and calls the relevant function
A detailed description of each call can be found at: http://www.msextra.com/doc/ms1extra/COM_RS232.htm
*/
void ts_communication()
{
    VU32 data_1, data_2, data_3, data_4;



    /**
    get a new command when ts_command_duration has expired
    */
    if(TS_cli.command_duration == 0)
    {
        TS_cli.cmdPending= 0;
    }

    if( !TS_cli.cmdPending )
    {
        TS_cli.currentCommand= UART_getRX();
        TS_cli.command_duration= COMMAND_DURATION;
    }


    switch(TS_cli.currentCommand)
    {

        case 'A':
            /**
            send 74 bytes of realtime values
            actually CAN part is fake
            */
            ts_sendValues(0, SENDVALUE_FAKE_PACKETSIZE);
            break;


        case 'B':
            /**
            Burn current configuration data to eeprom
            if you have permission to do so ;)
            */
            if(TS_cli.burn_permission == 0)
            {
                UART_Send(DEBUG_PORT, "\r\n*** config write rejected ***\r\n");
            }
            else
            {
                write_ConfigData();
            }

            break;


        case 'C':
            /**
            test communication
            This is used by Tunerstudio to see whether
            there is an ECU on a given serial port
            */
            UART_Tx(TS_PORT, '1');
            break;


        case 'D':

            /**
            received debug feature command
            */
            TS_cli.cmdPending = TRUE;

            if(UART_available() >= 2)
            {
                /**
                format:
                MSB, LSB
                */
                data_1= UART_getRX();
                data_2= UART_getRX();

                //run the desired feature
                ts_debug_features( word(data_1, data_2) );

                TS_cli.cmdPending = FALSE;
            }
            break;


        case 'F':
                /**
                send serial protocol version
                */
                UART_Send(TS_PORT, "001");
                break;


        case 'J':

                /**
                user permission management
                */
                TS_cli.cmdPending= TRUE;

                if(UART_available() >= 4)
                {
                    //read input
                    data_1= UART_getRX();
                    data_2= UART_getRX();
                    data_3= UART_getRX();
                    data_4= UART_getRX();

                    //get modification permission
                    if((data_1 == 'm') && (data_2 == 'o') && (data_3 == 'd') && (data_4 == '#'))
                    {
                        TS_cli.mod_permission =1;
                        UART_Send(DEBUG_PORT, "\r\n *** unlocked config modification ***");
                    }
                    else if((data_1 == 'b') && (data_2 == 'r') && (data_3 == 'n') && (data_4 == '!'))
                    {
                        TS_cli.burn_permission =1;
                        UART_Send(DEBUG_PORT, "\r\n *** unlocked config burn ***");
                    }

                    //ready
                    TS_cli.cmdPending = FALSE;
                }

            break;


        case 'L':
            /**
            List the contents of current page in human readable form
            */
            ts_diagPage();
            break;


        case 'N':
            /**
            Displays a new line.
            */
            UART_Send(TS_PORT, "\r\n");
            break;


        case 'P':

            /**
            set the current page
            (A 2nd byte of data is required after the 'P' specifying the new page number)
            */
            TS_cli.cmdPending= TRUE;

            if( UART_available() )
            {
                data_1= UART_getRX();

                /**
                This converts the ascii number char into binary.
                */
                if( data_1 >= 0x30 )
                {
                    data_1 -= 0x30;
                }

                if((data_1 > 0) && (data_1 < 13))
                {
                    TS_cli.currentPage_Nr= data_1;
                }

                TS_cli.cmdPending = FALSE;
            }

            break;


            case 'Q':
                /**
                send code version
                */
                UART_Send(TS_PORT, "speeduino 201708");
                break;


            case 'S':
                /**
                send code version
                */
                UART_Send(TS_PORT, "Speeduino 2017.08");

                //This is required in TS3 due to its stricter timings
                Tuareg.secl = 0;
                break;


            case 'V':
                /**
                send config data in binary
                */
                ts_sendPage();
                break;


            case 'W':
                /**
                receive new config value at 'W'+<offset>+<newbyte>
                */

                TS_cli.cmdPending = TRUE;

                if(TS_cli.currentPage_Nr == CALIBPAGE_NR)
                {
                    /**
                    this is for the calibration page:
                    we are expecting
                    16 bit calibration data
                    8 bit offset
                    */

                    if(UART_available() >= 3)
                    {
                        /**
                        offset
                        */
                        data_1= UART_getRX();


                        /**
                        value
                        MSB, LSB
                        */
                        data_2= UART_getRX();
                        data_3= UART_getRX();

                        //take the received data to config
                        ts_replaceConfig(data_1, word(data_2, data_3));

                        TS_cli.cmdPending = FALSE;
                    }
                }
                else if( (TS_cli.currentPage_Nr == VEMAPPAGE_NR) || (TS_cli.currentPage_Nr == IGNMAPPAGE_NR) || (TS_cli.currentPage_Nr == AFRMAPPAGE_NR) )
                {
                    /**
                    this is a map
                    */

                    if(UART_available() >= 3)
                    {
                        /**
                        offset
                        LSB, MSB
                        */
                        data_1= UART_getRX();
                        data_2= UART_getRX();

                        /**
                        value
                        */
                        data_3= UART_getRX();

                        //take the received data to config
                        ts_replaceConfig(word(data_2, data_1), data_3);

                        TS_cli.cmdPending = FALSE;
                    }
                }
                else
                {
                    if(UART_available() >= 2)
                    {
                        /**
                        offset
                        */
                        data_1= UART_getRX();

                        /**
                        value
                        */
                        data_2= UART_getRX();

                        //take the received data to config
                        ts_replaceConfig(data_1, data_2);

                        TS_cli.cmdPending = FALSE;
                    }
                }

                break;


            case 'r':
                /**
                New format for the optimized OutputChannels
                */
                TS_cli.cmdPending = TRUE;


                if (UART_available() >= 6)
                {
                    /**
                    byte 1
                    this should be $tsCanId
                    but it is currently not in use
                    */
                    data_1= UART_getRX();

                    /**
                    byte 2
                    cmd
                    */
                    data_2= UART_getRX();

                    //read the command byte
                    if(data_2 == 0x30)
                    {
                        /**
                        Send output channels
                        (command 0x30)
                        */

                        /**
                        byte 3 + 4
                        offset LSB, MSB
                        */
                        data_1= UART_getRX();
                        data_2= UART_getRX();

                        /**
                        byte 5 + 6
                        length LSB, MSB
                        */
                        data_3= UART_getRX();
                        data_4= UART_getRX();

                        //send live values
                        ts_sendValues(word(data_2, data_1), word(data_4, data_3));
                    }

                    TS_cli.cmdPending = FALSE;
                }
                break;

        case '?':

            UART_Tx(TS_PORT, '\n');
            UART_Send(TS_PORT, "===Command Help===\n\r");
            UART_Send(TS_PORT, "All commands are single character and are concatenated with their parameters \n\r");
            UART_Send(TS_PORT, "without spaces. Some parameters are binary and cannot be entered through this \n\r");
            UART_Send(TS_PORT, "prompt by conventional means. \n\r");
            UART_Send(TS_PORT, "Syntax:  <command>+<parameter1>+<parameter2>+<parameterN>\n\r");
            UART_Send(TS_PORT, "===List of Commands===\n\r");
            UART_Send(TS_PORT, "A - Displays 31 bytes of Tuareg values in binary (live data)\n\r");
            UART_Send(TS_PORT, "B - Burn current map and configPage values to eeprom\n\r");
            UART_Send(TS_PORT, "C - Test COM port.  Used by Tunerstudio to see whether an ECU is on a given serial \n\r");
            UART_Send(TS_PORT, "    port. Returns a binary number.\n\r");
            UART_Send(TS_PORT, "L - Displays map page (aka table) or configPage values.  Use P to change page (not \n\r");
            UART_Send(TS_PORT, "    every page is a map)\n\r");
            UART_Send(TS_PORT, "J - get permissions <mod#> or <brn!> \n\r");
            UART_Send(TS_PORT, "N - Print new line.\n\r");
            UART_Send(TS_PORT, "P - Set current page.  Syntax:  P+<pageNumber>\n\r");
            UART_Send(TS_PORT, "R - Same as A command\n\r");
            UART_Send(TS_PORT, "S - Display signature number\n\r");
            UART_Send(TS_PORT, "Q - Same as S command\n\r");
            UART_Send(TS_PORT, "V - Display map or configPage values in binary\n\r");
            UART_Send(TS_PORT, "W - Set one byte in map or configPage.  Expects binary parameters. \n\r");
            UART_Send(TS_PORT, "    Syntax:  W+<offset>+<new byte>\n\r");
            UART_Send(TS_PORT, "? - Displays this help page\n\r");

            break;

        default:
            break;
  }
}




/**
This function returns the current values of a fixed group of variables

a detailed view on the logs has revealed that the A command is not in use
by TunerStudio, mostly r (offset=0, length=31)
*/
void ts_sendValues(U32 offset, U32 length)
{
    U8 fullStatus[SENDVALUE_BUFFERSIZE];
    U32 i;

    if(TS_cli.A_cmd_requests == 0)
    {
        Tuareg.secl = 0;
    }

    TS_cli.A_cmd_requests++;

    fullStatus[0] = Tuareg.secl; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    fullStatus[1] = Tuareg.squirt; //Squirt Bitfield
    fullStatus[2] = Tuareg.engine; //Engine Status Bitfield
    fullStatus[3] = (U8)(Tuareg.ignition_timing.dwell_advance - Tuareg.ignition_timing.ignition_advance); //Dwell in ms * 10
    fullStatus[4] = lowByte(Tuareg.sensor_interface->MAP / 10); //2 U8s for MAP
    fullStatus[5] = highByte(Tuareg.sensor_interface->MAP / 10);
    fullStatus[6] = (U8)(Tuareg.sensor_interface->IAT + IAT_OFFSET); //mat
    fullStatus[7] = (U8)(Tuareg.sensor_interface->CLT + CLT_OFFSET); //Coolant ADC
    fullStatus[8] = (U8) Tuareg.sensor_interface->VBAT; //Battery voltage correction (%)
    fullStatus[9] = (U8) Tuareg.sensor_interface->VBAT; //battery voltage
    fullStatus[10] = (U8) Tuareg.sensor_interface->O2; //O2
    fullStatus[11] = Tuareg.egoCorrection; //Exhaust gas correction (%)
    fullStatus[12] = Tuareg.iatCorrection; //Air temperature Correction (%)
    fullStatus[13] = Tuareg.wueCorrection; //Warmup enrichment (%)
    fullStatus[14] = lowByte(Tuareg.decoder->engine_rpm); //rpm HB
    fullStatus[15] = highByte(Tuareg.decoder->engine_rpm); //rpm LB
    fullStatus[16] = Tuareg.TAEamount; //acceleration enrichment (%)
    fullStatus[17] = Tuareg.corrections; //Total GammaE (%)
    fullStatus[18] = Tuareg.VE; //Current VE 1 (%)
    fullStatus[19] = Tuareg.afrTarget;
    fullStatus[20] = (U8)(Tuareg.PW1 / 100); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    fullStatus[21] = (U8) Tuareg.sensor_interface->ddt_TPS; //TPS DOT
    fullStatus[22] = (U8) Tuareg.ignition_timing.ignition_advance;
    fullStatus[23] = (U8) Tuareg.sensor_interface->TPS; // TPS (0% to 100%)

    //Need to split the int loopsPerSecond value into 2 bytes
    fullStatus[24] = lowByte(Tuareg.loopsPerSecond);
    fullStatus[25] = highByte(Tuareg.loopsPerSecond);

    //The following can be used to show the amount of free memory
    //not needed by now
    fullStatus[26] = 0x42; //lowByte(Tuareg.freeRAM);
    fullStatus[27] = 0x42; //highByte(Tuareg.freeRAM);

    fullStatus[28] = 0x42; //boost target not needed
    fullStatus[29] = 0x42; //boost duty not needed
    fullStatus[30] = Tuareg.spark; //Spark related bitfield

    //rpmDOT must be sent as a signed integer
    fullStatus[31] = lowByte(Tuareg.rpmDOT);
    fullStatus[32] = highByte(Tuareg.rpmDOT);

    if(length > 31)
    {
        fullStatus[33] = 0x42; //Tuareg.ethanolPct; //Flex sensor value (or 0 if not used)
        fullStatus[34] = 0x42; //Tuareg.flexCorrection; //Flex fuel correction (% above or below 100)
        fullStatus[35] = 0x42; //Tuareg.flexIgnCorrection; //Ignition correction (Increased degrees of advance) for flex fuel
        fullStatus[36] = 0x42; //getNextError();

        fullStatus[37] = 0x42; //Tuareg.idleLoad;
        fullStatus[38] = 0x42; //Tuareg.testOutputs;

        fullStatus[39] = (U8) Tuareg.sensor_interface->O2; //O2
        fullStatus[40] = (U8) Tuareg.sensor_interface->BARO; //Barometer value

        /**
        we do not use CAN
        but message bytes 41 to 73 are expected
        to be about CAN
        by now
        */

        fullStatus[41] = (U8) Tuareg.sensor_interface->TPS;
    }

    /**
    print the requested section
    but avoid memory access violation
    */

    if(length > SENDVALUE_FAKE_PACKETSIZE)
    {
        length= SENDVALUE_FAKE_PACKETSIZE;
    }

    if((offset + length) > SENDVALUE_FAKE_PACKETSIZE)
    {
        offset= SENDVALUE_FAKE_PACKETSIZE - length;
    }


    /**
    i is number of sent bytes


    take care for fake CAN section

    TODO
    once CAN has been removed we can
    ease this procedure a lot
    */
    for(i=0; i < length; i++)
    {
        if( (i + offset) < 41)
        {
            //live data
            UART_Tx(TS_PORT, fullStatus[i + offset]);
        }
        else if( (i + offset) < 74)
        {
            //this is the fake CAN section
            UART_Tx(TS_PORT, 0);
        }
        else if( (i + offset) == 74 )
        {
            //TPS
            UART_Tx(TS_PORT, fullStatus[41]);
        }
    }
}


/**
this seems to be a nice diag mode feature

TODO
implement it!
*/
void ts_debug_features(U32 feature)
{
    switch (feature)
    {
        case 256:
            /**
            cmd is stop
            */

            /**
            TODO
            fill in something pretty here
            */

            /**
            dump eeprom
            U8 eeprom_data;
            for(data_1=0; data_1 < 4000; data_1++)
            {
                data_2= eeprom_read_byte(data_1, &eeprom_data);

                UART_Send(TS_PORT, "\r\n");
                UART_Print_U(TS_PORT, data_1, TYPE_U32, NO_PAD);

                if(data_2 == 0)
                {
                    UART_Print_U8Hex_new(TS_PORT, eeprom_data);
                }
                else
                {
                    UART_Tx(TS_PORT, '-');
                }

            }
            */

            /**
            //DEBUG timed sensor printout
        if( ls_timer & BIT_TIMER_1HZ)
        {
            ls_timer &= ~BIT_TIMER_1HZ;


            //intro
            UART_Send(DEBUG_PORT, "\r\nsensors:");

            if(hw_sensors->active_sensors & ASENSOR_MAP_ACT)
            {
                //injected conversion complete
                UART_Send(DEBUG_PORT, "\r\nMAP:");
                UART_Print_U(DEBUG_PORT, hw_sensors->MAP, TYPE_U16, NO_PAD);
            }

            if(hw_sensors->Intake_Vacuum)
            {
                UART_Send(DEBUG_PORT, "\r\nIntake vacuum:");
                UART_Print_U(DEBUG_PORT, hw_sensors->Intake_Vacuum, TYPE_U16, NO_PAD);
            }

            if(hw_sensors->active_sensors & ASENSOR_BARO_ACT)
            {
                UART_Send(DEBUG_PORT, "\r\nBARO:");
                UART_Print_U(DEBUG_PORT, hw_sensors->BARO, TYPE_U16, NO_PAD);
            }

            if(hw_sensors->active_sensors & ASENSOR_O2_ACT)
            {
                UART_Send(DEBUG_PORT, "\r\nO2:");
                UART_Print_U(DEBUG_PORT, hw_sensors->O2, TYPE_U16, NO_PAD);
            }

            if(hw_sensors->active_sensors & ASENSOR_TPS_ACT)
            {
                UART_Send(DEBUG_PORT, "\r\nTPS:");
                UART_Print_U(DEBUG_PORT, hw_sensors->TPS, TYPE_U16, NO_PAD);

                UART_Send(DEBUG_PORT, "\r\nddt_TPS:");
                UART_Print_S(DEBUG_PORT, hw_sensors->ddt_TPS, TYPE_S16, NO_PAD);
            }

            if(hw_sensors->active_sensors & ASENSOR_IAT_ACT)
            {
                UART_Send(DEBUG_PORT, "\r\nIAT:");
                UART_Print_U(DEBUG_PORT, hw_sensors->IAT, TYPE_U16, NO_PAD);
            }

            if(hw_sensors->active_sensors & ASENSOR_CLT_ACT)
            {
                UART_Send(DEBUG_PORT, "\r\nCLT:");
                UART_Print_U(DEBUG_PORT, hw_sensors->CLT, TYPE_U16, NO_PAD);
            }

            if(hw_sensors->active_sensors & ASENSOR_VBAT_ACT)
            {
                UART_Send(DEBUG_PORT, "\r\nVBAT:");
                UART_Print_U(DEBUG_PORT, hw_sensors->VBAT, TYPE_U16, NO_PAD);
            }

            //digital sensors
            UART_Send(DEBUG_PORT, "\r\nRUN, CRASH, SIDEST: ");

            if(hw_sensors->digital_sensors & DSENSOR_RUN)
            {
                UART_Tx(DEBUG_PORT, '1');
            }
            else
            {
                UART_Tx(DEBUG_PORT, '0');
            }

            UART_Tx(DEBUG_PORT, '-');

            if(hw_sensors->digital_sensors & DSENSOR_CRASH)
            {
                UART_Tx(DEBUG_PORT, '1');
            }
            else
            {
                UART_Tx(DEBUG_PORT, '0');
            }

            UART_Tx(DEBUG_PORT, '-');

            if(hw_sensors->digital_sensors & DSENSOR_SIDESTAND)
            {
                UART_Tx(DEBUG_PORT, '1');
            }
            else
            {
                UART_Tx(DEBUG_PORT, '0');
            }

        }
        */

            break;



    default:
      break;
  }
}




/**
ts_sendPage() packs the data within the current page (As set with the 'P' command)
into a buffer and sends it.
Note that some translation of the data is required to lay it out in the way Megasquirt / TunerStudio expect it
TunerStudio expects data as raw values, if you want human readable output - use ts_diagPage()
*/
void ts_sendPage()
{
    volatile void* pnt_configPage;
    U32 configPage_size;
    volatile table3D * currentTable;
    U8 response[MAP_PAGE_SIZE];
    U32 i, l;


    switch (TS_cli.currentPage_Nr)
    {

        case VEMAPPAGE_NR:
            currentTable = &fuelTable;
            break;

        case VESETPAGE_NR:
            pnt_configPage= &configPage1;
            configPage_size= VESETPAGE_SIZE;
            break;

        case IGNMAPPAGE_NR:
            currentTable = &ignitionTable;
            break;

        case IGNSETPAGE_NR:
            pnt_configPage = &configPage2;
            configPage_size= IGNSETPAGE_SIZE;
            break;

        case AFRMAPPAGE_NR:
            currentTable = &afrTable;
            break;

        case AFRSETPAGE_NR:
            pnt_configPage = &configPage3;
            configPage_size= AFRSETPAGE_SIZE;
            break;

        case IACPAGE_NR:
            pnt_configPage = &configPage4;
            configPage_size= IACPAGE_SIZE;
            break;


        case BOOSTVVCPAGE_NR:

            /**
            Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
            the size is: (8x8 + 8 + 8) + (8x8 + 8 + 8) = 160
            */

            //Boost table
            for (i = 0; i < 64; i++) { response[i] = boostTable.axisZ[(i / 8)][i % 8]; }
            for (i = 64; i < 72; i++) { response[i] = (U8) (boostTable.axisX[(i - 64)] / TABLE_RPM_MULTIPLIER); }
            for (i = 72; i < 80; i++) { response[i] = (U8) (boostTable.axisY[7 - (i - 72)]); }

            //VVT table
            for (i = 0; i < 64; i++) { response[i + 80] = vvtTable.axisZ[(i / 8)][i % 8]; }
            for (i = 64; i < 72; i++) { response[i + 80] = (U8) (vvtTable.axisX[(i - 64)] / TABLE_RPM_MULTIPLIER); }
            for (i = 72; i < 80; i++) { response[i + 80] = (U8) (vvtTable.axisY[7 - (i - 72)]); }

            //transmit via serial
            for (i = 0; i < BOOSTVVCPAGE_SIZE; i++)
            {
                UART_Tx(TS_PORT, response[i]);
            }

            return;

            break;


        case SEQFUELPAGE_NR:

            /**
            Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
            the size is: (6x6 + 6 + 6) * 4 = 192
            */

            //trim1 table
            for (i= 0; i < 36; i++) { response[i] = trim1Table.axisZ[(i / 6)][i % 6]; }
            for (i = 36; i < 42; i++) { response[i] = (U8) (trim1Table.axisX[(i - 36)] / TABLE_RPM_MULTIPLIER); }
            for (i = 42; i < 48; i++) { response[i] = (U8) (trim1Table.axisY[5 - (i - 42)] / TABLE_LOAD_MULTIPLIER); }

            //trim2 table
            for (i = 0; i < 36; i++) { response[i + 48] = trim2Table.axisZ[i / 6][i % 6]; }
            for (i = 36; i < 42; i++) { response[i + 48] = (U8) (trim2Table.axisX[(i - 36)] / TABLE_RPM_MULTIPLIER); }
            for (i = 42; i < 48; i++) { response[i + 48] = (U8) (trim2Table.axisY[5 - (i - 42)] / TABLE_LOAD_MULTIPLIER); }

            //trim3 table
            for (i = 0; i < 36; i++) { response[i + 96] = trim3Table.axisZ[(i / 6)][i % 6]; }
            for (i = 36; i < 42; i++) { response[i + 96] = (U8) (trim3Table.axisX[(i - 36)] / TABLE_RPM_MULTIPLIER); }
            for (i = 42; i < 48; i++) { response[i + 96] = (U8) (trim3Table.axisY[5 - (i - 42)] / TABLE_LOAD_MULTIPLIER); }

            //trim4 table
            for (i = 0; i < 36; i++) { response[i + 144] = trim4Table.axisZ[(i / 6)][i % 6]; }
            for (i = 36; i < 42; i++) { response[i + 144] = (U8) (trim4Table.axisX[(i - 36)] / TABLE_RPM_MULTIPLIER); }
            for (i = 42; i < 48; i++) { response[i + 144] = (U8) (trim4Table.axisY[5 - (i - 42)] / TABLE_LOAD_MULTIPLIER); }


            //transmit via serial
            for (i = 0; i < SEQFUELPAGE_SIZE; i++)
            {
                UART_Tx(TS_PORT, response[i]);
            }

            return;

            break;


        case CANBUSPAGE_NR:
            pnt_configPage = &configPage10;
            configPage_size= CANBUSPAGE_SIZE;
            break;

        case WARMUPPAGE_NR:
            pnt_configPage = &configPage11;
            configPage_size= WARMUPPAGE_SIZE;
            break;

        default:
            UART_Send(TS_PORT, "\n Page has not been implemented yet. Change to another page.");

            /**
            lets trick the output logic
            */
            pnt_configPage= &configPage1;
            configPage_size= 0;
            break;
    }


    if ( (TS_cli.currentPage_Nr == VEMAPPAGE_NR) || (TS_cli.currentPage_Nr == IGNMAPPAGE_NR) || (TS_cli.currentPage_Nr == AFRMAPPAGE_NR) )
    {
        /**
        this is a map
        */


        for(l= 0; l < 256; l++)
        {
            /**
            Z axis
            */
            response[l]= currentTable->axisZ[l / TABLE_3D_ARRAYSIZE][l % TABLE_3D_ARRAYSIZE];
        }

        for(l= 256; l < 272; l++)
        {
            //RPM Bins for VE table (Need to be divided by 100)
            response[l]= (U8) (currentTable->axisX[(l - 256)] / TABLE_RPM_MULTIPLIER);
        }

        for(l= 272; l < 288; l++)
        {
            //MAP or TPS bins for VE table
            response[l]= (U8) (currentTable->axisY[15 - (l - 272)] / TABLE_LOAD_MULTIPLIER);
        }

        /**
        send via serial
        */
        for (l= 0; l < MAP_PAGE_SIZE; l++)
        {
            UART_Tx(TS_PORT, response[l]);
        }

    }
    else
    {
        /**
        we have a config page to print
        */
        for (l= 0; l < configPage_size; l++)
        {
            /**
            Each byte is simply the location in memory of the
            configPage + the offset + the variable number (x)
            */
            UART_Tx(TS_PORT,  *((U8 *)pnt_configPage + l) );
        }
    }

}



/**
ts_diagPage() packs the data within the current page (As set with the 'P' command)
into a buffer and sends it in human readable form.
*/
void ts_diagPage()
{
    volatile table3D * currentTable;
    U32 sendComplete = FALSE;
    U32 x, i;
    U32 value;
    VU8 * currentVar;


    switch(TS_cli.currentPage_Nr)
    {
        case VEMAPPAGE_NR:
            currentTable = &fuelTable;
            UART_Send(TS_PORT, "\r \n Volumetric Efficiency Map \r \n");
            break;

        case VESETPAGE_NR:

            /**
            Display Values from Config Page 1
            */
            UART_Send(TS_PORT, "\r \n Page 1 Config \r \n");

            UART_Print_S(TS_PORT, configPage1.flexBoostLow, TYPE_S8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.flexBoostHigh, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.asePct, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.aseCount, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 10; i++)
            {
                UART_Print_U(TS_PORT, configPage1.wueValues[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage1.crankingPct, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.pinMapping, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.tachoPin, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.tdePct, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.taeColdA, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.tpsThresh, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.taeTime, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.displayType, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.display3, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.displayB1, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage1.reqFuel, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.divider, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.injTiming, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.injOpen, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.inj1Ang, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.inj2Ang, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.inj3Ang, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.inj4Ang, TYPE_U16, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage1.mapSample, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.cltType1, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.engineType, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.primePulse, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.dutyLim, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.flexFreqLow, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.flexFreqHigh, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.boostMaxDuty, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.tpsMin, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.tpsMax, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_S(TS_PORT, configPage1.mapMin, TYPE_S8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.mapMax, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.fpPrime, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.stoich, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.oddfire2, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.oddfire3, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.oddfire4, TYPE_U16, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage1.flexFuelLow, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.flexFuelHigh, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.flexAdvLow, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.flexAdvHigh, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.iacCLminDuty, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.iacCLmaxDuty, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage1.boostMinDuty, TYPE_U8, NO_PAD);

            sendComplete = TRUE;
            break;


        case IGNMAPPAGE_NR:
            currentTable = &ignitionTable;
            UART_Send(TS_PORT, "\r \n Ignition Map \r \n");
            break;

        case IGNSETPAGE_NR:

            /**
            Display Values from Config Page 2
            */
            UART_Send(TS_PORT, "\r \n Page 2 Config \r \n");

            UART_Print_U(TS_PORT, configPage2.triggerAngle, TYPE_U16, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.FixAng, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.CrankAng, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.TrigAngMul, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.TrigEdge, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.TrigEdgeSec, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.sparkDur, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.IdleAdvRPM, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.IdleAdvCLT, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.IdleDelayTime, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage2.StgCycles, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.dwellCont, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.dwellCrank, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.dwellRun, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.triggerTeeth, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.triggerMissingTeeth, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.crankRPM, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.floodClear, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.SoftRevLim, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.SoftLimRetard, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage2.SoftLimMax, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.HardRevLim, TYPE_U8, NO_PAD);

            for (i= 0; i < 4; i++)
            {
                UART_Print_U(TS_PORT, configPage2.taeBins[i], TYPE_U8, NO_PAD);
            }

            for (i= 0; i < 4; i++)
            {
                UART_Print_U(TS_PORT, configPage2.taeValues[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 10; i++)
            {
                UART_Print_U(TS_PORT, configPage2.wueBins[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage2.dwellLimit, TYPE_U8, NO_PAD);

            for (i= 0; i < 6; i++)
            {
                UART_Print_U(TS_PORT, configPage2.dwellCorrectionValues[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 6; i++)
            {
                UART_Print_U(TS_PORT, configPage2.iatRetBins[i], TYPE_U8, NO_PAD);
            }

            for (i= 0; i < 6; i++)
            {
                UART_Print_U(TS_PORT, configPage2.iatRetValues[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage2.dfcoRPM, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.dfcoHyster, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.dfcoTPSThresh, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage2.ignBypassEnabled, TYPE_U8, NO_PAD);

            sendComplete = TRUE;
            break;

        case AFRMAPPAGE_NR:
            currentTable = &afrTable;
            UART_Send(TS_PORT, "\r \n Air/Fuel Ratio Map \r \n");
            break;

        case AFRSETPAGE_NR:

            /**
            Display Values from Config Page 3
            */
            UART_Send(TS_PORT, "\r \n Page 3 Config \r \n");

            UART_Print_U(TS_PORT, configPage3.egoAlgorithm, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoKP, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoKI, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoKD, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoTemp, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoCount, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoDelta, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoLimit, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.ego_min, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.ego_max, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage3.ego_sdelay, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoRPM, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.egoTPSMax, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.vvtPin, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.boostPin, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 6; i++)
            {
                UART_Print_U(TS_PORT, configPage3.voltageCorrectionBins[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 6; i++)
            {
                UART_Print_U(TS_PORT, configPage3.injVoltageCorrectionValues[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 9; i++)
            {
                UART_Print_U(TS_PORT, configPage3.airDenBins[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            for (i= 0; i < 9; i++)
            {
                UART_Print_U(TS_PORT, configPage3.airDenRates[i], TYPE_U8, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage3.boostFreq, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.vvtFreq, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.idleFreq, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.launchPin, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.lnchSoftLim, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.lnchRetard, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.lnchHardLim, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.lnchFuelAdd, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.idleKP, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.idleKI, TYPE_U8, NO_PAD);

            UART_Send(TS_PORT, "\r\n");

            UART_Print_U(TS_PORT, configPage3.idleKD, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.boostLimit, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.boostKP, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.boostKI, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.boostKD, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.lnchPullRes, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.flatSSoftWin, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.flatSRetard, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage3.flatSArm, TYPE_U8, NO_PAD);

            sendComplete = TRUE;
            break;

        case IACPAGE_NR:

            /**
            Display Values from Config Page 4
            */
            UART_Send(TS_PORT, "\r \n Page 4 Config \r \n");

            /**
            Display four equally sized arrays at once
            */
            for (i= 4; i; i--)
            {

                switch(i)
                {
                    case 1: currentVar = configPage4.iacBins;
                            break;
                    case 2: currentVar = configPage4.iacOLPWMVal;
                            break;
                    case 3: currentVar = configPage4.iacOLStepVal;
                            break;
                    case 4: currentVar = configPage4.iacCLValues;
                            break;
                    default: break;
                }

                for(x = 10; x; x--)
                {
                    UART_Print_U(TS_PORT, currentVar[10 - x], TYPE_U8, NO_PAD);
                }

                UART_Send(TS_PORT, "\r\n");
            }

            // Three equally sized arrays
            for (i= 3; i; i--)
            {
                switch (i)
                {
                    case 1: currentVar = configPage4.iacCrankBins;
                            break;
                    case 2: currentVar = configPage4.iacCrankDuty;
                            break;
                    case 3: currentVar = configPage4.iacCrankSteps;
                            break;
                    default: break;
                }

                for (x= 4; x; x--)
                {
                    UART_Print_U(TS_PORT, currentVar[4 - x], TYPE_U8, NO_PAD);
                }

                UART_Send(TS_PORT, "\r\n");
            }

            UART_Print_U(TS_PORT, configPage4.iacAlgorithm, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.iacFastTemp, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.iacStepHome, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.iacStepHyster, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.fanInv, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.fanSP, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.fanHyster, TYPE_U8, NO_PAD);
            UART_Print_U(TS_PORT, configPage4.fanFreq, TYPE_U8, NO_PAD);

            for (i= 0; i < 4; i++)
            {
                UART_Print_U(TS_PORT, configPage4.fanPWMBins[i], TYPE_U8, NO_PAD);
            }

            sendComplete = TRUE;
            break;


        case BOOSTVVCPAGE_NR:
                currentTable= &boostTable;
                UART_Send(TS_PORT, "\r \n Boost Map \r \n");
                break;


        case SEQFUELPAGE_NR:

                currentTable= &trim1Table;
                UART_Send(TS_PORT, "\r \n VVT Map \r \n");

                for (i = 0; i < currentTable->dimension; i++)
                {
                    value= (U8) (currentTable->axisY[i]);
                    UART_Print_U(TS_PORT, value, TYPE_U8, NO_PAD);

                    for (x = 0; x < currentTable->dimension; x++)
                    {
                        value= currentTable->axisZ[i][x];
                        UART_Print_U(TS_PORT, value, TYPE_U8, NO_PAD);
                    }

                    UART_Send(TS_PORT, "\r\n");
                }

                sendComplete = TRUE;
                break;

        case CALIBPAGE_NR:

                /**
                Display Values from Config Page 9
                */
                UART_Send(TS_PORT, "\r \n Sensor calibration: \r \n");

                /**
                IAT
                */
                UART_Send(TS_PORT, "IAT: \r \nx: ");

                for(i=0; i< CALIBRATION_TABLE_DIMENSION; i++)
                {
                    UART_Print_U(TS_PORT, configPage9.IAT_calib_data_x[i], TYPE_U16, PAD);
                }

                UART_Send(TS_PORT, "\r \ny: ");

                for(i=0; i< CALIBRATION_TABLE_DIMENSION; i++)
                {
                    UART_Print_U(TS_PORT, configPage9.IAT_calib_data_y[i], TYPE_U16, PAD);
                }


                /**
                CLT
                */
                UART_Send(TS_PORT, "\r\n CLT: \r\nx: ");

                for(i=0; i< CALIBRATION_TABLE_DIMENSION; i++)
                {
                    UART_Print_U(TS_PORT, configPage9.CLT_calib_data_x[i], TYPE_U16, PAD);
                }

                UART_Send(TS_PORT, "\r \ny: ");

                for(i=0; i< CALIBRATION_TABLE_DIMENSION; i++)
                {
                    UART_Print_U(TS_PORT, configPage9.CLT_calib_data_y[i], TYPE_U16, PAD);
                }


                /**
                TPS
                */
                UART_Send(TS_PORT, "\r\n TPS: \r\nx: ");

                for(i=0; i< CALIBRATION_TABLE_DIMENSION; i++)
                {
                    UART_Print_U(TS_PORT, configPage9.TPS_calib_data_x[i], TYPE_U16, PAD);
                }

                UART_Send(TS_PORT, "\r \ny: ");

                for(i=0; i< CALIBRATION_TABLE_DIMENSION; i++)
                {
                    UART_Print_U(TS_PORT, configPage9.TPS_calib_data_y[i], TYPE_U16, PAD);
                }

                /**
                MAP
                */
                UART_Send(TS_PORT, "\r \n MAP: \r\nM: ");
                UART_Print_U(TS_PORT, configPage9.MAP_calib_M, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "N: ");
                UART_Print_U(TS_PORT, configPage9.MAP_calib_N, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "L: ");
                UART_Print_U(TS_PORT, configPage9.MAP_calib_L, TYPE_U16, NO_PAD);

                /**
                BARO
                */
                UART_Send(TS_PORT, "\r \n BARO: \r\nM: ");
                UART_Print_U(TS_PORT, configPage9.BARO_calib_M, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "N: ");
                UART_Print_U(TS_PORT, configPage9.BARO_calib_N, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "L: ");
                UART_Print_U(TS_PORT, configPage9.BARO_calib_L, TYPE_U16, NO_PAD);

                /**
                O2
                */
                UART_Send(TS_PORT, "\r \n O2: \r\nM: ");
                UART_Print_U(TS_PORT, configPage9.O2_calib_M, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "N: ");
                UART_Print_U(TS_PORT, configPage9.O2_calib_N, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "L: ");
                UART_Print_U(TS_PORT, configPage9.O2_calib_L, TYPE_U16, NO_PAD);

                /**
                VBAT
                */
                UART_Send(TS_PORT, "\r \n VBAT: \r\nM: ");
                UART_Print_U(TS_PORT, configPage9.VBAT_calib_M, TYPE_U16, NO_PAD);
                UART_Send(TS_PORT, "L: ");
                UART_Print_U(TS_PORT, configPage9.VBAT_calib_L, TYPE_U16, NO_PAD);

                sendComplete = TRUE;
                break;

        default:
            UART_Send(TS_PORT, "\n Page has not been implemented yet. Change to another page.");
            sendComplete = TRUE;
            break;
    }

    if(!sendComplete)
    {
            /**
            title already printed
            */

            for (i= 0; i < TABLE_3D_ARRAYSIZE; i++)
            {
                UART_Print_U(TS_PORT, currentTable->axisY[TABLE_3D_ARRAYSIZE -1 - i], TYPE_U16, NO_PAD);

                for (x = 0; x < currentTable->dimension; x++)
                {
                    UART_Print_U(TS_PORT, currentTable->axisZ[TABLE_3D_ARRAYSIZE -1 - i][x], TYPE_U16, NO_PAD);
                }

                UART_Send(TS_PORT, "\r\n");
            }

            UART_Send(TS_PORT, "    ");

            // Horizontal bins
            for (x = 0; x < currentTable->dimension; x++)
            {
                UART_Print_U(TS_PORT, currentTable->axisX[x] / 100, TYPE_U16, NO_PAD);
            }

            UART_Send(TS_PORT, "\r\n");

    }

}


/**
a byte sent by TunerStudio shall replace our
current configuration value
*/
void ts_replaceConfig(U32 valueOffset, U32 newValue)
{

    volatile void* pnt_configPage;
    U32 tempOffset;

    if(TS_cli.mod_permission == 0)
    {
        UART_Send(DEBUG_PORT, "\r\n*** config modification rejected ***\r\n");
        return;
    }

    switch(TS_cli.currentPage_Nr)
    {
        case VEMAPPAGE_NR:

            if (valueOffset < 256)
            {
                fuelTable.axisZ[(valueOffset / 16)][valueOffset % 16] = newValue;
            }
            else if (valueOffset < 272)
            {
                /**
                X Axis
                The RPM values sent by megasquirt are divided by 100,
                need to multiple it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                fuelTable.axisX[(valueOffset - 256)] = (U16)(newValue * TABLE_RPM_MULTIPLIER);
            }
            else
            {
                /**
                Y Axis
                Need to do a translation to flip the order
                (Due to us using (0,0) in the top left rather than bottom right
                */
                tempOffset = 15 - (valueOffset - 272);
                fuelTable.axisY[tempOffset] = (U16)(newValue) * TABLE_LOAD_MULTIPLIER;
            }

            break;

        case VESETPAGE_NR:

            pnt_configPage = &configPage1;

            /**
            For some reason, TunerStudio is sending offsets greater than the maximum page size.
            I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
            */
            if (valueOffset < PAGE_SIZE)
            {
                *((U8 *)pnt_configPage + (U8)valueOffset) = newValue;
            }
            break;


        case IGNMAPPAGE_NR:

            if (valueOffset < 256)
            {
                //New value is part of the ignition map
                ignitionTable.axisZ[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
            }
            else if (valueOffset < 272)
            {
                /**
                X Axis
                The RPM values sent by megasquirt are divided by 100,
                need to multiple it back by 100 to make it correct
                */
                ignitionTable.axisX[(valueOffset - 256)] = (S16)(newValue) * TABLE_RPM_MULTIPLIER;
            }
            else
            {
                /**
                Y Axis
                Need to do a translation to flip the order
                */
                tempOffset = 15 - (valueOffset - 272);
                ignitionTable.axisY[tempOffset] = (S16)(newValue) * TABLE_LOAD_MULTIPLIER;
            }

            break;

        case IGNSETPAGE_NR:

            pnt_configPage = &configPage2;

            /**
            For some reason, TunerStudio is sending offsets greater than the maximum page size.
            I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
            */
            if (valueOffset < PAGE_SIZE)
            {
                *((U8 *)pnt_configPage + (U8)valueOffset) = newValue;
            }
            break;

        case AFRMAPPAGE_NR:

            if (valueOffset < 256)
            {
                //New value is part of the afr map
                afrTable.axisZ[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
            }
            else if (valueOffset < 272)
            {
                /**
                X Axis
                The RPM values sent by megasquirt are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                afrTable.axisX[(valueOffset - 256)] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else
            {
                /**
                Y Axis
                Need to do a translation to flip the order
                */
                tempOffset = 15 - (valueOffset - 272);
                afrTable.axisY[tempOffset] = (S16) newValue * TABLE_LOAD_MULTIPLIER;
            }
            break;


        case AFRSETPAGE_NR:
            pnt_configPage = &configPage3;

            /**
            For some reason, TunerStudio is sending offsets greater than the maximum page size.
            I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
            */
            if (valueOffset < PAGE_SIZE)
            {
                *((U8 *)pnt_configPage + (U8)valueOffset) = newValue;
            }
            break;


        case IACPAGE_NR:
            pnt_configPage = &configPage4;

            /**
            For some reason, TunerStudio is sending offsets greater than the maximum page size.
            I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
            */
            if (valueOffset < PAGE_SIZE)
            {
                *((U8 *)pnt_configPage + (U8)valueOffset) = newValue;
            }
            break;

        case BOOSTVVCPAGE_NR:

            /**
            Boost and VVT maps (8x8)
            */
            if (valueOffset < 64)
            {
                //New value is part of the boost map
                boostTable.axisZ[7 - (valueOffset / 8)][valueOffset % 8] = newValue;
            }
            else if (valueOffset < 72)
            {
                /**
                New value is on the X (RPM) axis of the boost table
                The RPM values sent by TunerStudio are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                boostTable.axisX[(valueOffset - 64)] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else if (valueOffset < 80)
            {
                /**
                New value is on the Y (TPS) axis of the boost table
                TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
                */
                boostTable.axisY[(7 - (valueOffset - 72))] = (S16) newValue;
            }
            else if (valueOffset < 144)
            {
                //New value is part of the vvt map
                tempOffset = valueOffset - 80;
                vvtTable.axisZ[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
            }
            else if (valueOffset < 152)
            {
                /**
                New value is on the X (RPM) axis of the vvt table
                The RPM values sent by TunerStudio are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                tempOffset = valueOffset - 144;
                vvtTable.axisX[tempOffset] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else
            {
                /**
                New value is on the Y (Load) axis of the vvt table
                TABLE_LOAD_MULTIPLIER is NOT used for vvt as it is TPS based (0-100)
                */
                tempOffset = valueOffset - 152;
                vvtTable.axisY[(7 - tempOffset)] = (S16) newValue;
            }

            break;

        case SEQFUELPAGE_NR:
        {
            if (valueOffset < 36)
            {
                //Trim1 values
                trim1Table.axisZ[5 - (valueOffset / 6)][valueOffset % 6] = newValue;
            }
            else if (valueOffset < 42)
            {
                /**
                New value is on the X (RPM) axis of the trim1 table.
                The RPM values sent by TunerStudio are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                trim1Table.axisX[(valueOffset - 36)] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else if (valueOffset < 48)
            {
                trim1Table.axisY[(5 - (valueOffset - 42))] = (S16) newValue * TABLE_LOAD_MULTIPLIER;
            }
            else if (valueOffset < 84)
            {
                //New value is part of the trim2 map
                tempOffset = valueOffset - 48; trim2Table.axisZ[5 - (tempOffset / 6)][tempOffset % 6] = newValue;
            }
            else if (valueOffset < 90)
            {
                /**
                New value is on the X (RPM) axis of the table.
                The RPM values sent by TunerStudio are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                tempOffset = valueOffset - 84;
                trim2Table.axisX[tempOffset] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else if (valueOffset < 96)
            {
                //New value is on the Y (Load) axis of the table
                tempOffset = valueOffset - 90;
                trim2Table.axisY[(5 - tempOffset)] = (S16) newValue * TABLE_LOAD_MULTIPLIER;
            }
            else if (valueOffset < 132)
            {
                //New value is part of the trim3 map
                tempOffset = valueOffset - 96; trim3Table.axisZ[5 - (tempOffset / 6)][tempOffset % 6] = newValue;
            }
            else if (valueOffset < 138)
            {
                /**
                New value is on the X (RPM) axis of the table.
                The RPM values sent by TunerStudio are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                tempOffset = valueOffset - 132; trim3Table.axisX[tempOffset] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else if (valueOffset < 144)
            {
                //New value is on the Y (Load) axis of the table
                tempOffset = valueOffset - 138; trim3Table.axisY[(5 - tempOffset)] = (S16) newValue * TABLE_LOAD_MULTIPLIER;
            }
            else if (valueOffset < 180)
            {
                //New value is part of the trim2 map
                tempOffset = valueOffset - 144;
                trim4Table.axisZ[5 - (tempOffset / 6)][tempOffset % 6] = newValue;
            }
            else if (valueOffset < 186)
            {
                /**
                New value is on the X (RPM) axis of the table.
                The RPM values sent by TunerStudio are divided by 100,
                need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
                */
                tempOffset = valueOffset - 180;
                trim4Table.axisX[tempOffset] = (S16) newValue * TABLE_RPM_MULTIPLIER;
            }
            else if (valueOffset < 192)
            {
                //New value is on the Y (Load) axis of the table
                tempOffset = valueOffset - 186;
                trim4Table.axisY[(5 - tempOffset)] = (S16) newValue * TABLE_LOAD_MULTIPLIER;
            }
        }

        break;


        case WARMUPPAGE_NR:
            pnt_configPage = &configPage11;

            /**
            For some reason, TunerStudio is sending offsets greater than the maximum page size.
            I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
            */
            if (valueOffset < PAGE_SIZE)
            {
                *((U8 *)pnt_configPage + (U8)valueOffset) = newValue;
            }

            break;


        case CALIBPAGE_NR:

            /**
            TODO
            reformat with generic literals
            IAT x [6]
                y [6]
            CLT x [6]
                y [6]
            TPS x [6]
                y [6]
            MAP m
                n
                l
            BARO
                m
                n
                l
            */
            if(valueOffset < CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.IAT_calib_data_x[valueOffset]= newValue;
            }
            else if(valueOffset < 2* CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.IAT_calib_data_y[valueOffset - CALIBRATION_TABLE_DIMENSION]= newValue;
            }
            else if(valueOffset < 3* CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.CLT_calib_data_x[valueOffset - 2* CALIBRATION_TABLE_DIMENSION]= newValue;
            }
            else if(valueOffset < 4* CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.CLT_calib_data_y[valueOffset - 3* CALIBRATION_TABLE_DIMENSION]= newValue;
            }
            else if(valueOffset < 5* CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.TPS_calib_data_x[valueOffset - 4* CALIBRATION_TABLE_DIMENSION]= newValue;
            }
            else if(valueOffset < 6* CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.TPS_calib_data_y[valueOffset - 5* CALIBRATION_TABLE_DIMENSION]= newValue;
            }
            //MAP
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION)
            {
                configPage9.MAP_calib_M= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +1)
            {
                configPage9.MAP_calib_N= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +2)
            {
                configPage9.MAP_calib_L= newValue;
            }
            //BARO
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +3)
            {
                configPage9.BARO_calib_M= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +4)
            {
                configPage9.BARO_calib_N= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +5)
            {
                configPage9.BARO_calib_L= newValue;
            }
            //O2
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +6)
            {
                configPage9.O2_calib_M= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +7)
            {
                configPage9.O2_calib_N= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +8)
            {
                configPage9.O2_calib_L= newValue;
            }
            //VBAT
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +9)
            {
                configPage9.VBAT_calib_M= newValue;
            }
            else if(valueOffset == 6* CALIBRATION_TABLE_DIMENSION +10)
            {
                configPage9.VBAT_calib_L= newValue;
            }


            break;


        default:
            break;
  }
}

