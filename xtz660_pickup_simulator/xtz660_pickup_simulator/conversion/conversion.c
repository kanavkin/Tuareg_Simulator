#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "../types.h"



void CV_U8Char(U8 value, char * Target)
{
    char * cv_ziffer;
    cv_ziffer= Target;

    U8 hunderter;
    U8 zehner;


    for(hunderter=0; value > 99; hunderter++)
    {
        value -= 100;
    }

    for(zehner=0; value > 9; zehner++)
    {
        value -= 10;
    }

    //Werte in ASCII
    *cv_ziffer=(hunderter + 0x30);
    cv_ziffer++;

    *cv_ziffer=(zehner + 0x30);
    cv_ziffer++;

    *cv_ziffer=(value + 0x30);

}

void CV_U16Char(U16 value, char * Target, U8 term_string, U8 nozero)
{
    char * cv_ziffer;
    cv_ziffer= Target;

    U8 zehntausend;
    U8 tausend;
    U8 hunderter;
    U8 zehner;
    U8 lead_zero= 0xFF;

    for(zehntausend=0; value > 9999; zehntausend++)
    {
        value -= 10000;
    }

    for(tausend=0; value > 999; tausend++)
    {
        value -= 1000;
    }

    for(hunderter=0; value > 99; hunderter++)
    {
        value -= 100;
    }

    for(zehner=0; value > 9; zehner++)
    {
        value -= 10;
    }

    //ASCII
    if((zehntausend == 0) && nozero)
    {
        //space
        *cv_ziffer= 0x20;
    }
    else
    {
        *cv_ziffer=(zehntausend + 0x30);
    }

    if(zehntausend)
    {
        lead_zero= 0;
    }

    cv_ziffer++;


    if((tausend == 0) && nozero && lead_zero)
    {
        //space
        *cv_ziffer= 0x20;
    }
    else
    {
        *cv_ziffer=(tausend + 0x30);
    }

    if(tausend)
    {
        lead_zero= 0;
    }

    cv_ziffer++;

    if((hunderter == 0) && nozero && lead_zero)
    {
        //space
        *cv_ziffer= 0x20;
    }
    else
    {
        *cv_ziffer=(hunderter + 0x30);
    }

    if(hunderter)
    {
        lead_zero= 0;
    }

    cv_ziffer++;

    if((zehner == 0) && nozero && lead_zero)
    {
        //space
        *cv_ziffer= 0x20;
    }
    else
    {
        *cv_ziffer=(zehner + 0x30);
    }

    cv_ziffer++;

    //einer
    *cv_ziffer=(value + 0x30);


    if(term_string != 0)
    {
        //terminate string
        cv_ziffer++;
        *cv_ziffer= 0x00;
    }

}
