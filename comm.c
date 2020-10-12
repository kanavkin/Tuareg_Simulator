/**
TODO
- implement diag buttons
    (ts_commandButtons)
*/




#include "utils.h"
#include "table.h"

#include "uart.h"
#include "conversion.h"
#include "comm.h"

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
void comm_periodic()
{
    VU32 i;
    VU16 number;
    VU8 cmd_in[COMMAND_CMD_LEN];
    VU8 num_in[COMMAND_NUM_LEN];


    /**
    get a new command when ts_command_duration has expired
    */
    if(TS_cli.command_duration == 0)
    {
        TS_cli.currentCommand= CMD_IDLE;
    }


    //nothing to do if no new rx data is available
    if( UART_available() == 0)
    {
        return;
    }

    //read next command in
    if(TS_cli.currentCommand == CMD_IDLE)
    {

        if(UART_available() >= COMMAND_CMD_LEN)
        {
            //get cmd part
            for(i=0; i< COMMAND_CMD_LEN; i++)
            {
                cmd_in[i]= UART_getRX();
            }


            if((cmd_in[0] == 'c') && (cmd_in[1] == 'o') && (cmd_in[2] == 'n') && (cmd_in[3] == 't') && (cmd_in[4] == 'i'))
            {
                TS_cli.currentCommand= CMD_CONT_MODE;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 's') && (cmd_in[1] == 'w') && (cmd_in[2] == 'e') && (cmd_in[3] == 'e') && (cmd_in[4] == 'p'))
            {
                TS_cli.currentCommand= CMD_SWEEP_MODE;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 's') && (cmd_in[1] == 'w') && (cmd_in[2] == 'b') && (cmd_in[3] == 'e') && (cmd_in[4] == 'g'))
            {
                TS_cli.currentCommand= CMD_SWEEP_START;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 's') && (cmd_in[1] == 'w') && (cmd_in[2] == 'e') && (cmd_in[3] == 'n') && (cmd_in[4] == 'd'))
            {
                TS_cli.currentCommand= CMD_SWEEP_END;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 's') && (cmd_in[1] == 'w') && (cmd_in[2] == 'i') && (cmd_in[3] == 'n') && (cmd_in[4] == 'c'))
            {
                TS_cli.currentCommand= CMD_SWEEP_INCREMENT;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 's') && (cmd_in[1] == 'w') && (cmd_in[2] == 'h') && (cmd_in[3] == 'l') && (cmd_in[4] == 'd'))
            {
                TS_cli.currentCommand= CMD_SWEEP_HOLD;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 'c') && (cmd_in[1] == 'o') && (cmd_in[2] == 'r') && (cmd_in[3] == 'p') && (cmd_in[4] == 'm'))
            {
                TS_cli.currentCommand= CMD_CONT_RPM;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else if((cmd_in[0] == 'b') && (cmd_in[1] == 'r') && (cmd_in[2] == 'e') && (cmd_in[3] == 'a') && (cmd_in[4] == 'k'))
            {
                TS_cli.currentCommand= CMD_BREAK;
                TS_cli.command_duration= COMMAND_DURATION;
            }
            else
            {
                UART_Send(TS_PORT, "\r\n\r\n===Command Help===");
                UART_Send(TS_PORT, "\r\nCommand format: 5 characters, followed by 5 numbers if required");
                UART_Send(TS_PORT, "\r\nconti - enable continuous mode");
                UART_Send(TS_PORT, "\r\nsweep - enable sweep mode");
                UART_Send(TS_PORT, "\r\nswbeg<numbr> - set sweep initial rpm");
                UART_Send(TS_PORT, "\r\nswend<numbr> - set sweep final rpm");
                UART_Send(TS_PORT, "\r\nswinc<numbr> - set sweep rpm increment");
                UART_Send(TS_PORT, "\r\nswhld<numbr> - set sweep hold value");
                UART_Send(TS_PORT, "\r\ncorpm<numbr> - set rpm for continuous mode\r\n\r\n");
            }

        }

    }


    //get additional data if needed and run the command
    switch(TS_cli.currentCommand)
    {

    case CMD_BREAK:

        stop_crank_simulation();
        UART_Send(TS_PORT, "\r\n*** STOP ***");

        //ready
        TS_cli.currentCommand= CMD_IDLE;
        TS_cli.command_duration =0;

        break;

    case CMD_CONT_MODE:

        set_continuous_mode();

        //ready
        TS_cli.currentCommand= CMD_IDLE;
        TS_cli.command_duration =0;

        break;

    case CMD_SWEEP_MODE:

        set_sweep_mode();

        //ready
        TS_cli.currentCommand= CMD_IDLE;
        TS_cli.command_duration =0;

        break;

    case CMD_SWEEP_START:

        if(UART_available() >= COMMAND_NUM_LEN)
        {
            //get numeric part
            for(i=0; i< COMMAND_NUM_LEN; i++)
            {
                num_in[i]= UART_getRX();
            }

            //extract numeric part
            number= (num_in[0] - 0x30) * 10000 + (num_in[1] - 0x30) * 1000 + (num_in[2] - 0x30) * 100 + (num_in[3] - 0x30) * 10 + (num_in[4] - 0x30);

            if(number < MAX_RPM)
            {
                Tuareg_simulator.pCrank_simulator->sweep_start= number;

                UART_Send(TS_PORT, "\r\n setting sweep start to: ");
                UART_Print_U(TS_PORT, number, TYPE_U16, NO_PAD);
            }

            //ready
            TS_cli.currentCommand= CMD_IDLE;
            TS_cli.command_duration =0;

        }
        break;

    case CMD_SWEEP_END:

        if(UART_available() >= COMMAND_NUM_LEN)
        {
            //get numeric part
            for(i=0; i< COMMAND_NUM_LEN; i++)
            {
                num_in[i]= UART_getRX();
            }

            //extract numeric part
            number= (num_in[0] - 0x30) * 10000 + (num_in[1] - 0x30) * 1000 + (num_in[2] - 0x30) * 100 + (num_in[3] - 0x30) * 10 + (num_in[4] - 0x30);

            if((number <= MAX_RPM) && (number > 0) && (number > Tuareg_simulator.pCrank_simulator->sweep_start))
            {
                Tuareg_simulator.pCrank_simulator->sweep_end= number;

                UART_Send(TS_PORT, "\r\n setting sweep end to: ");
                UART_Print_U(TS_PORT, number, TYPE_U16, NO_PAD);
            }

            //ready
            TS_cli.currentCommand= CMD_IDLE;
            TS_cli.command_duration =0;

        }

        break;

    case CMD_SWEEP_INCREMENT:

        if(UART_available() >= COMMAND_NUM_LEN)
        {
            //get numeric part
            for(i=0; i< COMMAND_NUM_LEN; i++)
            {
                num_in[i]= UART_getRX();
            }

            //extract numeric part
            number= (num_in[0] - 0x30) * 10000 + (num_in[1] - 0x30) * 1000 + (num_in[2] - 0x30) * 100 + (num_in[3] - 0x30) * 10 + (num_in[4] - 0x30);

            if((number < MAX_RPM) && (number > 0))
            {
                Tuareg_simulator.pCrank_simulator->sweep_increment= number;

                UART_Send(TS_PORT, "\r\n setting sweep increment to: ");
                UART_Print_U(TS_PORT, number, TYPE_U16, NO_PAD);
            }

            //ready
            TS_cli.currentCommand= CMD_IDLE;
            TS_cli.command_duration =0;

        }

        break;

    case CMD_SWEEP_HOLD:

        if(UART_available() >= COMMAND_NUM_LEN)
        {
            //get numeric part
            for(i=0; i< COMMAND_NUM_LEN; i++)
            {
                num_in[i]= UART_getRX();
            }

            //extract numeric part
            number= (num_in[0] - 0x30) * 10000 + (num_in[1] - 0x30) * 1000 + (num_in[2] - 0x30) * 100 + (num_in[3] - 0x30) * 10 + (num_in[4] - 0x30);

            if(number < MAX_RPM)
            {
                Tuareg_simulator.pCrank_simulator->sweep_hold= number;

                UART_Send(TS_PORT, "\r\n set sweep hold to: ");
                UART_Print_U(TS_PORT, number, TYPE_U16, NO_PAD);
            }

            //ready
            TS_cli.currentCommand= CMD_IDLE;
            TS_cli.command_duration =0;

        }

        break;

    case CMD_CONT_RPM:

        if(UART_available() >= COMMAND_NUM_LEN)
        {
            //get numeric part
            for(i=0; i< COMMAND_NUM_LEN; i++)
            {
                num_in[i]= UART_getRX();
            }

            //extract numeric part
            number= (num_in[0] - 0x30) * 10000 + (num_in[1] - 0x30) * 1000 + (num_in[2] - 0x30) * 100 + (num_in[3] - 0x30) * 10 + (num_in[4] - 0x30);

            set_crank_rpm(number);

            if(number < MAX_RPM)
            {
                Tuareg_simulator.pCrank_simulator->cont_rpm= number;

                UART_Send(TS_PORT, "\r\n set cont rpm to: ");
                UART_Print_U(TS_PORT, Tuareg_simulator.pCrank_simulator->cont_rpm, TYPE_U16, NO_PAD);
            }

            //ready
            TS_cli.currentCommand= CMD_IDLE;
            TS_cli.command_duration =0;

        }

        break;

    default:
            //ready
            TS_cli.currentCommand= CMD_IDLE;
            TS_cli.command_duration =0;
        break;

    }
















}





