

#include "types.h"
#include "conversion.h"
#include "uart.h"

/**
needed for UART_Print functions:
puts one converted decimal place to desired uart port,
returns the new lead_zero value
*/
U32 uart_push_decimal_place(USART_TypeDef * Port, U32 number, U32 lead_zero, U32 padding)
{
    if(number)
    {
        //decimal place to print
        UART_Tx(Port, number + 0x30);

        //no longer leading zeros
        return 0;
    }
    else
    {
        if(lead_zero)
        {
            if(padding)
            {
                UART_Tx(Port, ' ');
            }

        }
        else
        {
            UART_Tx(Port, '0');
        }

        //lead_zero unchanged
        return lead_zero;
    }
}



void UART_Print_S(USART_TypeDef * Port, S32 value, conversion_int_t inttype, U32 padding)
{
    switch(inttype)
    {
        case TYPE_S32:
            inttype= TYPE_U32;
            break;

        case TYPE_S16:
            inttype= TYPE_U16;
            break;

        case TYPE_S8:
            inttype= TYPE_U8;
            break;

        default:
            return;

    }

    if(value < 0)
    {
        UART_Tx(Port, '-');
        UART_Print_U(Port, -value, inttype, padding);
    }
    else
    {
        UART_Print_U(Port, value, inttype, padding);
    }
}


/**
0xFFFFFFFF = 4 294 967 295
*/
void UART_Print_U(USART_TypeDef * Port, U32 value, conversion_int_t inttype, U32 padding)
{
    U32 number;
    U32 lead_zero= 0xFFFFFFFF;

    switch(inttype)
    {
        case TYPE_U32:

                //milliard
                for(number=0; value > 999999999; number++)
                {
                    value -= 1000000000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                //hundred millions
                for(number=0; value > 99999999; number++)
                {
                    value -= 100000000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                //ten millions
                for(number=0; value > 9999999; number++)
                {
                    value -= 10000000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                //millions
                for(number=0; value > 999999; number++)
                {
                    value -= 1000000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                //hundred thousands
                for(number=0; value > 99999; number++)
                {
                    value -= 100000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                /**
                fall through
                */

        case TYPE_U16:

                //ten thousands
                for(number=0; value > 9999; number++)
                {
                    value -= 10000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                //thousands
                for(number=0; value > 999; number++)
                {
                    value -= 1000;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                /**
                fall through
                */

        case TYPE_U8:

                //hundert
                for(number=0; value > 99; number++)
                {
                    value -= 100;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                //ten
                for(number=0; value > 9; number++)
                {
                    value -= 10;
                }

                lead_zero= uart_push_decimal_place(Port, number, lead_zero, padding);

                /**
                print remainder
                */
                UART_Tx(Port, value + 0x30);


                /**
                print trailing space
                */
                UART_Tx(Port, ' ');

                break;

        default:
            return;


    }

}


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

/*
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

    if((ten == 0) && (hun == 0))
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
*/



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


/*
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

    if((ten == 0) && (hun == 0))
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
*/


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


/*
void UART_Print_U16(U16 value)
{
    U32 number;
    U32 lead_zero;

    //ten thousands
    for(number=0; value > 9999; number++)
    {
        value -= 10000;
    }

    if(number)
    {
        UART_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {

        UART_Tx(' ');
        lead_zero= 0xFFFFFFFF;

    }

    //thousands
    for(number=0; value > 999; number++)
    {
        value -= 1000;
    }

    if(number)
    {
        UART_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART_Tx(' ');
        }
        else
        {
            UART_Tx('0');
        }

    }

    //hundert
    for(number=0; value > 99; number++)
    {
        value -= 100;
    }

    if(number)
    {
        UART_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART_Tx(' ');
        }
        else
        {
            UART_Tx('0');
        }

    }

    //ten
    for(number=0; value > 9; number++)
    {
        value -= 10;
    }

    if(number)
    {
        UART_Tx(number + 0x30);
    }
    else
    {
        if(lead_zero)
        {
            UART_Tx(' ');
        }
        else
        {
            UART_Tx('0');
        }

    }

    //remainder
    UART_Tx(value + 0x30);


    //trailing space
    UART_Tx(' ');
}
*/


/**
0xFFFFFFFF = 4 294 967 295
*/
/*
void UART1_Print_U32(U32 value)
{
    U32 number;
    U32 lead_zero;


    //milliard
    for(number=0; value > 999999999; number++)
    {
        value -= 1000000000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0UL;
    }
    else
    {
        UART1_Tx(' ');
        lead_zero= 0xFFFFFFFF;
    }

    //hundred millions
    for(number=0; value > 99999999; number++)
    {
        value -= 100000000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //ten millions
    for(number=0; value > 9999999; number++)
    {
        value -= 10000000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //millions
    for(number=0; value > 999999; number++)
    {
        value -= 1000000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //hundred thousands
    for(number=0; value > 99999; number++)
    {
        value -= 100000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //ten thousands
    for(number=0; value > 9999; number++)
    {
        value -= 10000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //thousands
    for(number=0; value > 999; number++)
    {
        value -= 1000;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //hundert
    for(number=0; value > 99; number++)
    {
        value -= 100;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
        lead_zero= 0x00;
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //ten
    for(number=0; value > 9; number++)
    {
        value -= 10;
    }

    if(number)
    {
        UART1_Tx(number + 0x30);
    }
    else
    {
        if(lead_zero)
        {
            UART1_Tx(' ');
        }
        else
        {
            UART1_Tx('0');
        }

    }

    //remainder
    UART1_Tx(value + 0x30);


    //trailing space
    UART1_Tx(' ');
}
*/







void UART_Print_U8Hex(USART_TypeDef * Port, U8 value)
{
    U32 nibble;

    UART_Send(Port, "0x");

    //high nibble
    nibble=((value >> 4) & 0x0F);

    if(nibble < 10)
    {
        UART_Tx(Port, nibble + 0x30 );
    }
    else
    {
        // 'A' := 65
        UART_Tx(Port, nibble + 55);
    }

    //low nibble
    nibble=(value & 0x0F);

    if(nibble < 10)
    {
        UART_Tx(Port, nibble + 0x30 );
    }
    else
    {
        // 'A' := 65
        UART_Tx(Port, nibble + 55);
    }

    //trailing space
    UART_Tx(Port,' ');
}



void UART_Print_U8Hex_new(USART_TypeDef * Port, U8 value)
{
    const char digits[]= "0123456789ABCDEF";

    UART_Send(Port, "0x");

    //high nibble
    UART_Tx(Port, digits[(value >> 4) & 0x0f] );

    //low nibble
    UART_Tx(Port, digits[(value) & 0x0f] );

    //trailing space
    UART_Tx(Port, ' ');
}

