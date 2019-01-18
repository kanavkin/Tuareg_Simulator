/*
These are some utility functions and variables used through the main code
*/
#ifndef UTILS_H
#define UTILS_H

#include "types.h"

//Handy bitsetting macros
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_CHECK(var,pos) !!((var) & (1<<(pos)))

void EnterCrit();
void LeaveCrit();

U16 word(U8 high, U8 low);
U8 lowByte(U16);
U8 highByte(U16);

U16 freeRam();
void initialiseTriggers();
void setPinMapping(U8);

#endif // UTILS_H
