#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "types.h"
#include "ignition.h"

typedef struct
{
  __IO uint32_t CTRL;                        /*!< Offset: 0x00  Control Register    */
  __IO uint32_t CYCCNT;                        /*!< Offset: 0x04  Cycle counter Register        */

} SWT_Type;

/* Core Debug registers */
#define DWT_BASE    (0xE0001000)
#define DWT         ((SWT_Type *) DWT_BASE)



void init_debug_pins();
void set_debug_pin( output_pin_t level);
void set_debug_led( output_pin_t level);

void print_full_state(volatile ignition_timing_t * intime);
void print_minimal_state(USART_TypeDef * Port, volatile ignition_timing_t * intime);


void dwt_init();
void delay_us(U32 delay);


#endif // DEBUG_H_INCLUDED
