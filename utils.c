/**
  Speeduino - Simple engine management for the Arduino Mega 2560 platform
  Copyright (C) Josh Stewart
  A full copy of the license may be found in the projects root directory


  TODO
  EnterCrit() seems to crash


*/



#include "types.h"


#include "utils.h"

#include "config.h"


U8 triggerInterrupt; // By default, use the first interrupt
U8 triggerInterrupt2;




U8 global_Sreg_Copy;
U8 pgm_is_critical;

/*
  Returns how much free dynamic memory exists (between heap and stack)
  This function is one big MISRA violation.
  MISRA advisories forbid directly poking at memory addresses, however there is no other way of determining heap size on embedded systems.
*/
uint16_t freeRam ()
{
    /*
  extern int __heap_start, *__brkval;
  uint16_t v;

  return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  */
  return 0;
}


/*
EnterCrit()
*/
void EnterCrit()
{
    /*
    if( !pgm_is_critical )
    {
        global_Sreg_Copy=SREG;
        cli();
        pgm_is_critical= 0xFF;
    }
    */
}

void LeaveCrit()
{
    /*
    if( pgm_is_critical )
    {
        SREG=global_Sreg_Copy;
        pgm_is_critical= 0;
        sei();
    }
    */
}



U8 lowByte(U16 in)
{
    return (U8)(in & 0x00FF);
}

U8 highByte(U16 in)
{
    return (U8)(in >> 8);
}

U16 word(U8 high, U8 low)
{
    return ((high << 8) | low);
}













