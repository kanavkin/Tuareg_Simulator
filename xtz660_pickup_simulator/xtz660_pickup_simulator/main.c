/**
XTZ 660 ignition simulator
generates crank pickup signal with 16 bit timer 1
generates dwell and ignition with 8 bit timer 0


*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "types.h"
#include "uart.h"
#include "conversion.h"
#include "simulator.h"
#include "profiles.h"

//#include <avr/iom328p.h>



enum {

    POSITION_A1= 0,
    POSITION_A2= 1,
    POSITION_B1= 2,
    POSITION_B2= 3,
    POSITION_C1= 4,
    POSITION_C2= 5,
    POSITION_D1= 6,
    POSITION_D2= 7,
    UNDEFINED_POSITION= 0xFF
};

// holds the timer values for the next crank turn
volatile U16 timer_segments[8];

// holds the segments lengths for the next crank turn
volatile U8 crank_segments[8]={CRANK_SEGMENT_A, CRANK_SEGMENT_B, CRANK_SEGMENT_C, CRANK_SEGMENT_D, CRANK_SEGMENT_E, CRANK_SEGMENT_F, CRANK_SEGMENT_G, CRANK_SEGMENT_H};

volatile U8 engine_position= POSITION_D2;
volatile U16 engine_rpm= ENGINE_CRANKING_RPM;

//serial comm
char serial_input[6];
VU8 serial_count;

/**
flag indicating full crank turn
triggers timing recalculation
needed in init to be == 1
*/
volatile U8 cycle_end= 1;
volatile U32 cycle_counter;

volatile U8 worker_pointer;
volatile U8 verbose_counter;
volatile char verbose_buffer[10];

VU8 run_profile;


/**
ignition stuff
characteristics hard coded to this tables!
*/
volatile U8 coil_on_timing, coil_off_timing, coil_on_pos, coil_off_pos, ignition_advance;
volatile U16 dwell_advance, dwell_buffer;

U16 dwell_table[]={0, 15, 31, 46, 61, 77, 92, 108, 123, 138, 154, 169, 184, 200, 215, 230, 246, 261, 276};
U8 adv_table[]={0, 3, 12, 12, 12, 12, 24, 25, 28, 30, 31, 33, 35, 35, 35, 38, 38, 38, 38};



/**
calculate the duration (in timer ticks) of a sement at given rpm
*/
U16 calc_segment_time(U8 length, U16 rpm)
{
    return 333333UL * length / rpm;
}


/**
calculate the duration (in timer 0 ticks) of an advance angle
*/
U8 calc_advance_time(U16 angle, U16 rpm)
{
    dwell_buffer= 10417UL * angle / rpm;

    if(dwell_buffer > 255)
    {
        return 255;
    }
    else
    {
        return dwell_buffer;
    }
}


/**
return the advance (in deg) to a given rpm
*/
U8 get_advance(U16 rpm)
{
    return adv_table[rpm >> 9];
}


/**
return dwell time (in deg) to a given rpm from table
*/
U16 get_dwell(U16 rpm)
{
    return dwell_table[rpm >> 9];
}


/**
calculates the best position and the resulting timing to fit the given delay time
for dwell and ignition advance
due to the 8 bit timer the advancing capabilities are limited at low revs!
take this into account when using this function!!!
*/
void fit_position(volatile U16 rpm, volatile U16 advance, volatile U8 * to_position, volatile U8 * to_timing)
{
    if(advance == POSITION_B2_ADVANCE)
    {
        // 0°
        * to_position= POSITION_B2;
        * to_timing= 0;
    }
    else if(advance <= POSITION_B1_ADVANCE)
    {
        // 1° - 10°
        * to_position= POSITION_B1;
        * to_timing= calc_advance_time((POSITION_B1_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_B1_ADVANCE) && (advance <= POSITION_A2_ADVANCE))
    {
        // 11° - 60°
        * to_position= POSITION_A2;
        * to_timing= calc_advance_time((POSITION_A2_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_A2_ADVANCE) && (advance <= POSITION_A1_ADVANCE))
    {
        // 61° - 100°
        * to_position= POSITION_A1;
        * to_timing= calc_advance_time((POSITION_A1_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_A1_ADVANCE) && (advance <= POSITION_D2_ADVANCE))
    {
        // 101° - 185°
        * to_position= POSITION_D2;
        * to_timing= calc_advance_time((POSITION_D2_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_D2_ADVANCE) && (advance <= POSITION_D1_ADVANCE))
    {
        // 186° - 190°
        * to_position= POSITION_D1;
        * to_timing= calc_advance_time((POSITION_D1_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_D1_ADVANCE) && (advance <= POSITION_C2_ADVANCE))
    {
        // 191° - 275°
        * to_position= POSITION_C2;
        * to_timing= calc_advance_time((POSITION_C2_ADVANCE - advance), rpm);
    }
    else if((advance > POSITION_C2_ADVANCE) && (advance <= POSITION_C1_ADVANCE))
    {
        // 276° - 280°
        * to_position= POSITION_C1;
        * to_timing= calc_advance_time((POSITION_C1_ADVANCE - advance), rpm);
    }

    /**
    sanity check for 8 bit timer
    */
    if(* to_timing > 255)
    {
        *to_timing= 255;
    }
}



/**
calculates the trigger signals for the next engine cycle
at a given rpm
*/
void calc_engine_timings(U16 rpm)
{
    for(worker_pointer=0; worker_pointer < 8; worker_pointer++)
    {
        //calculate segment length
        timer_segments[worker_pointer]= calc_segment_time(crank_segments[worker_pointer], rpm);
    }

    /**
    take care for low rev limitations
    */
    if(rpm > DYNAMIC_MIN_RPM)
    {
        // calculate advance
        ignition_advance= get_advance(rpm);
        fit_position(rpm, ignition_advance, &coil_off_pos, &coil_off_timing );

        // calculate dwell
        dwell_advance= ignition_advance + get_dwell(rpm);
        fit_position(rpm, dwell_advance, &coil_on_pos, &coil_on_timing );
    }
    else if(rpm > LOWREV_MIN_RPM)
    {
        coil_on_pos= LOWREV_DWELL_POSITION;
        coil_off_pos= LOWREV_IGNITION_POSITION;
        ignition_advance= LOWREV_IGNITION_ADVANCE;
        coil_on_timing= 0;
        coil_off_timing= 0;
    }
    else
    {
        coil_on_pos= CRANKING_DWELL_POSITION;
        coil_off_pos= CRANKING_IGNITION_POSITION;
        ignition_advance= CRANKING_IGNITION_ADVANCE;
        coil_on_timing= 0;
        coil_off_timing= 0;
    }
}






void set_simulator_pin(U8 level)
{
    if(level == ON)
    {
        PORTD |= (1<< PORTD4);
    }
    else if(level == TOGGLE)
    {
        //never read the register when toggle!!!
        PIND= (1<< 4);
    }
    else
    {
        PORTD &= ~(1<< PORTD4);
    }
}


void set_trigger_pin(U8 level)
{
    if(level == ON)
    {
        PORTD |= (1<< PORTD7);
    }
    else if(level == TOGGLE)
    {
            PIND= (1<< 7);
    }
    else
    {
        PORTD &= ~(1<< PORTD7);
    }
}




void print_state()
{
    /**
            print engine information to serial
            */

            UART_Tx('\r');

            UART_Print_U16(engine_rpm);
            UART_Send_P(PSTR("rpm, advance: "));
            UART_Print_U8(ignition_advance);
            UART_Send_P(PSTR(" deg, dwell advance: "));
            UART_Print_U16(dwell_advance);

            UART_Send_P(PSTR(" deg, profile: "));
            UART_Print_U8(run_profile);

            UART_Send_P(PSTR(" timings: on: "));

            switch(coil_on_pos)
            {
            case POSITION_A1:
                UART_Send("A1");
                break;
            case POSITION_A2:
                UART_Send("A2");
                break;
                case POSITION_B1:
                UART_Send("B1");
                break;
                case POSITION_B2:
                UART_Send("B2");
                break;
                case POSITION_C1:
                UART_Send("C1");
                break;
                case POSITION_C2:
                UART_Send("C2");
                break;
                case POSITION_D1:
                UART_Send("D1");
                break;
                case POSITION_D2:
                UART_Send("D2");
                break;
            }

            UART_Tx('-');
            UART_Print_U8(coil_on_timing);

            UART_Send_P(PSTR("off:"));

            switch(coil_off_pos)
            {
            case POSITION_A1:
                UART_Send("A1");
                break;
            case POSITION_A2:
                UART_Send("A2");
                break;
                case POSITION_B1:
                UART_Send("B1");
                break;
                case POSITION_B2:
                UART_Send("B2");
                break;
                case POSITION_C1:
                UART_Send("C1");
                break;
                case POSITION_C2:
                UART_Send("C2");
                break;
                case POSITION_D1:
                UART_Send("D1");
                break;
                case POSITION_D2:
                UART_Send("D2");
                break;
            }

            UART_Tx('-');
            UART_Print_U8(coil_off_timing);
}



void serial_com()
{
    U16 number;

    if(serial_count == 0)
    {
        serial_input[0]= UART_getRX();

        if( (serial_input[0] == 'r') || (serial_input[0] == 'c') || (serial_input[0] == 'i') || (serial_input[0] == 'd') )
        {
            serial_count= 1;
        }
    }
    else if(serial_count < 6)
    {
            serial_input[serial_count]= UART_getRX();
            serial_count++;
    }
    else
    {
        //extract numeric part
        number= (serial_input[1] - 0x30) * 10000 + (serial_input[2] - 0x30) * 1000 + (serial_input[3] - 0x30) * 100 + (serial_input[4] - 0x30) * 10 + (serial_input[5] - 0x30);

        switch(serial_input[0])
        {
        case 'r':

            if( (run_profile == 0) && (number <= SIMULATOR_MAX_RPM))
            {
                engine_rpm= number;
            }

            break;

        case 'i':

            if( (run_profile == 0) && (engine_rpm + number <= SIMULATOR_MAX_RPM) )
            {
                engine_rpm += number;
            }

            break;


        case 'd':

            if( (run_profile == 0) && (engine_rpm - number <= SIMULATOR_MAX_RPM) )
            {
                engine_rpm -= number;
            }

            break;

        case 'c':

            if( (run_profile) && (serial_input[1] == 'n') && (serial_input[2] == 'o') && (serial_input[3] == 'p') && (serial_input[4] == 'r') && (serial_input[5] == 'o'))
            {
                //manual mode
                run_profile= 0;
            }
            else if( (serial_input[1] == 'p') && (serial_input[2] == 'r') && (serial_input[3] == 'o') && (serial_input[4] == 'f') )
            {
                //profile mode
                if((serial_input[5] >= '0') && (serial_input[5] <= '9'))
                {
                    run_profile= serial_input[5] - 0x30;
                }

                //restart profile
                cycle_counter=0;
                engine_rpm=0;

            }

            break;


        default:
            break;


        }

        serial_count =0;
    }
}





int main(void)
{

    /*
    set up simulation pins
    */
    DDRD= (1<< PIND4) | (1<< PIND7);
    set_simulator_pin(IDLE_SIGNAL);
    set_trigger_pin(OFF);

    /**
    enable timer1
    CTC mode, ps 8
    enable comp A Int
    */
    TCCR1B= (1<< WGM12) | (1<< CS11);
    TIFR1= 0x02;
    TIMSK1= 0x02;
    OCR1A= 0xFFFE;

    /**
    enable timer0
    CTC mode, ps 256
    enable comp A Int on demand
    */
    TCCR0A= (1<< WGM01);
    TCCR0B=(1<< CS02);
    OCR0A= 0xFF;

    //boot message
    UART_Init();
    UART_Send("\r \n \r \n . \r \n . \r \n . \r \n \r \n *** XTZ 660 crank pickup signal simulator booting ... *** \r \n");
    UART_Send("\r \n config: \r \n");
    UART_Send("XTZ 660 crank signal on PORTD4 \r \n");
    UART_Send("-PORTD4 is digital 4 on Arduino UNO- \r \n");
    UART_Send("\r \nXTZ 660 ignition coil signal on PORTD7 \r \n");
    UART_Send("-PORTD7 is digital 7 on Arduino UNO- \r \n \r \n");

    //default profile
    run_profile= 1;


    /**
    ... and off we go ...
    */
    sei();

    while(1)
    {

        /**
        after one full crank turn
        waiting for new timing information
        */
        if(cycle_end)
        {
            cycle_end= 0;
            cycle_counter++;

            run_rpm_profile(run_profile);

            print_state();
        }

        //look for new commands
        if( UART_available() )
        {
            serial_com();
        }

    }

    return 0;
}



/**

*/
ISR(TIMER1_COMPA_vect)
{
    if(engine_position == POSITION_D2)
    {
        engine_position= POSITION_A1;
        cycle_end= 1;
        set_simulator_pin(KEY_SIGNAL_POLARITY);
    }
    else
    {
        engine_position++;
        set_simulator_pin(TOGGLE);
    }

    OCR1A= timer_segments[engine_position];

    /**
    handle ignition coil interrupt
    for the whole engine cycle
    */
    if(engine_position == coil_on_pos)
    {
        if(coil_on_timing == 0)
        {
            // immediate trigger
            set_trigger_pin(ON);
        }
        else
        {
            // use timer0
            TCNT0= 0;
            OCR0A= coil_on_timing;
            TIFR0= 0x07;
            TIMSK0 |= 0x02;
        }
    }
    else if(engine_position == coil_off_pos)
    {
        if(coil_off_timing == 0)
        {
            // immediate trigger
            set_trigger_pin(OFF);
        }
        else
        {
            // use timer0
            TCNT0= 0;
            OCR0A= coil_off_timing;
            TIFR0= 0x07;
            TIMSK0 |= 0x02;
        }
    }
}

/**
ignition coil interrupt
*/
ISR(TIMER0_COMPA_vect)
{
    /**
    handle ignition coil
    */
    if(engine_position == coil_on_pos)
    {
        set_trigger_pin(ON);
    }
    else if(engine_position == coil_off_pos)
    {
        set_trigger_pin(OFF);
    }

    //disable interrupt
    OCR0A= 0xFF;
    TIMSK0 &= ~(1<< 1);
}
