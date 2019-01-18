#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "conversion.h"
#include "uart.h"


void CV_U8Char(U8 value, char * Target)
{
    U8 hun;
    U8 ten;

    for(hun=0; value > 99; hun++)
    {
        value -= 100;
    }

    for(ten=0; value > 9; ten++)
    {
        value -= 10;
    }

    //ASCII
    *Target=(hun + 0x30);
    Target++;

    *Target=(ten + 0x30);
    Target++;

    *Target=(value + 0x30);

}


void UART_Print_U8(U8 value)
{
    U8 hun;
    U8 ten;

    for(hun=0; value > 99; hun++)
    {
        value -= 100;
    }

    for(ten=0; value > 9; ten++)
    {
        value -= 10;
    }

    //ASCII
    if(hun == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(hun + 0x30);
    }

    if(ten == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(ten + 0x30);
    }

    UART_Tx(value + 0x30);

    //trailing space
    UART_Tx(' ');
}



void CV_S8Char(S8 value, char * Target)
{
    U8 hun;
    U8 ten;

    if(value < 0)
    {
        *Target= '-';
        value= (U8) -value;
    }
    else
    {
        *Target=' ';
        value= (U8) value;
    }

    Target++;

    for(hun=0; value > 99; hun++)
    {
        value -= 100;
    }

    for(ten=0; value > 9; ten++)
    {
        value -= 10;
    }

    //ASCII
    *Target=(hun + 0x30);
    Target++;

    *Target=(ten + 0x30);
    Target++;

    *Target=(value + 0x30);

}


void UART_Print_S8(S8 value)
{
    U8 hun;
    U8 ten;

    if(value < 0)
    {
        UART_Tx('-');
        value= (U8) -value;
    }
    else
    {
        UART_Tx(' ');
        value= (U8) value;
    }

    for(hun=0; value > 99; hun++)
    {
        value -= 100;
    }

    for(ten=0; value > 9; ten++)
    {
        value -= 10;
    }

    //ASCII
    if(hun == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(hun + 0x30);
    }

    if(ten == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(ten + 0x30);
    }

    UART_Tx(value + 0x30);

    //trailing space
    UART_Tx(' ');
}


void CV_U16Char(U16 value,  char * Target,  U8 term_string,  U8 nozero)
{
     U8 tent;
     U8 thous;
     U8 hun;
     U8 ten;
     U8 lead_zero= 0xFF;

    for(tent=0; value > 9999; tent++)
    {
        value -= 10000;
    }

    for(thous=0; value > 999; thous++)
    {
        value -= 1000;
    }

    for(hun=0; value > 99; hun++)
    {
        value -= 100;
    }

    for(ten=0; value > 9; ten++)
    {
        value -= 10;
    }

    //ASCII
    if((tent == 0) && nozero)
    {
        //space
        *Target= 0x20;
    }
    else
    {
        *Target=(tent + 0x30);
    }

    if(tent)
    {
        lead_zero= 0;
    }

    Target++;


    if((thous == 0) && nozero && lead_zero)
    {
        //space
        *Target= 0x20;
    }
    else
    {
        *Target=(thous + 0x30);
    }

    if(thous)
    {
        lead_zero= 0;
    }

    Target++;

    if((hun == 0) && nozero && lead_zero)
    {
        //space
        *Target= 0x20;
    }
    else
    {
        *Target=(hun + 0x30);
    }

    if(hun)
    {
        lead_zero= 0;
    }

    Target++;

    if((ten == 0) && nozero && lead_zero)
    {
        //space
        *Target= 0x20;
    }
    else
    {
        *Target=(ten + 0x30);
    }

    Target++;

    //einer
    *Target=(value + 0x30);


    if(term_string != 0)
    {
        //terminate string
        Target++;
        *Target= 0x00;
    }

}



void UART_Print_U16(U16 value)
{
    U8 tent;
    U8 thous;
    U8 hun;
    U8 ten;

    for(tent=0; value > 9999; tent++)
    {
        value -= 10000;
    }

    for(thous=0; value > 999; thous++)
    {
        value -= 1000;
    }

    for(hun=0; value > 99; hun++)
    {
        value -= 100;
    }

    for(ten=0; value > 9; ten++)
    {
        value -= 10;
    }

    //ASCII
    if(tent == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(tent + 0x30);
    }

    if(thous == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(thous + 0x30);
    }

    if(hun == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(hun + 0x30);
    }

    if(ten == 0)
    {
        UART_Tx(' ');
    }
    else
    {
        UART_Tx(ten + 0x30);
    }

    UART_Tx(value + 0x30);

    //trailing space
    UART_Tx(' ');
}

