/**
this module contains all the debug tool
that should never be integrated to a
production version
*/

#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"

#include "types.h"
#include "debug.h"
#include "decoder.h"
#include "ignition.h"
#include "uart.h"
#include "conversion.h"


volatile char print_buffer[10];


/**
dbug led on GPIOA-11
(this is led2 on nucleo f103rb)
*/
void set_debug_led(volatile output_pin_t level)
{
    if(level == ON)
    {
        //on
        GPIOA->BSRR= GPIO_BSRR_BS11;
    }
    else if(level == TOGGLE)
    {
        if(GPIOA->ODR & GPIO_ODR_ODR11)
        {
            // OFF
            GPIOA->BRR= GPIO_BRR_BR11;
        }
        else
        {
            //on
            GPIOA->BSRR= GPIO_BSRR_BS11;
        }
    }
    else
    {
        // OFF
        GPIOA->BRR= GPIO_BRR_BR11;
    }
}



/**
dbug pin on GPIOA-12
(this is D12 on nucleo f103rb)
*/
void set_debug_pin(output_pin_t level)
{
    if(level == ON)
    {
        //on
        GPIOA->BSRR= GPIO_BSRR_BS12;
    }
    else if(level == TOGGLE)
    {
        if(GPIOA->ODR & GPIO_ODR_ODR12)
        {
            // OFF
            GPIOA->BRR= GPIO_BRR_BR12;
        }
        else
        {
            //on
            GPIOA->BSRR= GPIO_BSRR_BS12;
        }
    }
    else
    {
        // OFF
        GPIOA->BRR= GPIO_BRR_BR12;
    }
}



void init_debug_pins()
{
    //Enable PORT A clock
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

    //set output mode
    GPIO_configure(GPIOA, 11, GPIO_OUT_PP_2MHZ);
    GPIO_configure(GPIOA, 12, GPIO_OUT_PP_2MHZ);

    //clear
    set_debug_led(OFF);
    set_debug_pin(OFF);
}






/**
full state is so heavy
that it destroys engine timing
print_full_state() takes about 855 us
*/
/*
void print_full_state(volatile ignition_timing_t * intime)
{
    UART_newline();

    UART_Print_U16(intime->rpm);
    UART_Send("rpm, advance: ");
    UART_Print_U8(intime->ignition_advance);
    UART_Send_P(PSTR(" deg, dwell advance: "));
    UART_Print_U16(intime->dwell_advance);
    UART_Send_P(PSTR(" deg, on: "));

            switch(intime->coil_on_pos)
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
                case UNDEFINED_POSITION:
                break;
            }

            UART_Tx('-');
            UART_Print_U8(intime->coil_on_timing);

            UART_Send_P(PSTR("off:"));

            switch(intime->coil_off_pos)
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
                case UNDEFINED_POSITION:
                break;
            }

            UART_Tx('-');
            UART_Print_U8(intime->coil_off_timing);
}
*/


void print_minimal_state(USART_TypeDef * Port, volatile ignition_timing_t * intime)
{
    UART_transmit(Port,'\r');
    UART_Print_U(Port, intime->rpm, TYPE_U32, NO_PAD);
    UART_write(Port, "rpm, advance ");
    UART_Print_U(Port, intime->ignition_advance, TYPE_U32, NO_PAD);
    UART_write(Port, "deg");
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











