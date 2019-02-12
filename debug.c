/**
this module contains all the debug tool
that should never be integrated to a
production version
*/

#include "stm32_libs/stm32f10x/stm32f10x.h"
#include "stm32_libs/stm32f10x/boctok/stm32f10x_gpio_boctok.h"
#include "stm32_libs/boctok_types.h"


#include "debug.h"

#include "ignition.h"
#include "uart.h"
#include "conversion.h"


volatile char print_buffer[10];


/**
dbug led on PORTC13
*/
void set_debug_led(volatile output_pin_t level)
{
    if(level == ON)
    {
        gpio_set_pin(GPIOC, 13, OFF);
    }
    else if(level == OFF)
    {
        gpio_set_pin(GPIOC, 13, ON);
    }
    else
    {
        gpio_set_pin(GPIOC, 13, TOGGLE);
    }

}


void init_debug_pins()
{
    //PORT.C13 is led

    //Enable PORTC clock
    RCC->APB2ENR |= RCC_APB2Periph_GPIOC;

    //set output mode
    GPIO_configure(GPIOC, 13, GPIO_OUT_OD_2MHZ);

    //clear
    set_debug_led(OFF);
}







void dwt_init()
{
    //if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT= 0;
        DWT->CTRL |= 0x01;
    }
}

void delay_us(U32 delay)
{
    U32 target;

    target= DWT->CYCCNT + delay * (SystemCoreClock / 1000000);

    while( DWT->CYCCNT < target);
}











