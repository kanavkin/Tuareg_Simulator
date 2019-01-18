#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>


#include "types.h"
#include "uart.h"

#include "conversion.h"


VU8 UART_buffer[UART_BUFFER_SIZE];
VU8 rx_ptr, rd_ptr;



void UART_Init()
{
    //baud rate 115200
	UBRR0H= 0;
	UBRR0L= 16;

	//double speed
	UCSR0A= (1<< U2X0);

	//uart module enabled, rx complete Int
	UCSR0B= (1<< RXEN0) | (1<< TXEN0) | (1<< RXCIE0);

	//no parity, 1 stop bit, 8 data bits
	UCSR0C= (1<< UCSZ01) | (1<< UCSZ00);

	rx_ptr =0;
	rd_ptr =0;
}


void UART_Tx(char msg)
{
		while (!(UCSR0A & (1<< UDRE0)))
		;

		UDR0= msg;
}


//send string
void UART_Send(char messg[] )
{
    for( ; *messg != 0 ; messg++)
    {
        UART_Tx( *messg );
    }
}



/**
send string from flash

example:
UART_Send_P(PSTR("FLASH STRING"));
*/
void UART_Send_P(const char *data)
{
	while (pgm_read_byte(data) != 0x00)
    {
        UART_Tx(pgm_read_byte(data++));
    }
}



void UART_newline()
{
    UART_Tx('\r');
    UART_Tx('\n');
}




void UART_reset()
{
    rx_ptr=0;
    rd_ptr=0;
}


void UART_nolisten()
{
    /**
    turn off Receive Complete Int
    */
    UCSR0B &= ~(1<< RXCIE0);
}


/**
notify how many bytes are in RX buffer
*/
U8 UART_available()
{
    if(rx_ptr > rd_ptr)
    {
        return rx_ptr - rd_ptr;
    }
    else if (rx_ptr < rd_ptr)
    {
        return (UART_BUFFER_SIZE -rd_ptr + rx_ptr);
    }
    else
    {
        return 0;
    }
}



/**
put one byte from RX buffer
*/
U8 UART_getRX()
{
    U8 out;
    //EnterCrit();

    if(rd_ptr == rx_ptr)
    {
        /**
        called but no data in buffer
        */
        return 0xFF;
    }
    else
    {
        out= UART_buffer[rd_ptr];
        rd_ptr++;

        if(rd_ptr == UART_BUFFER_SIZE)
        {
            rd_ptr =0;
        }

        return  out;
    }

    //LeaveCrit();
}


/**
debug uart buffer functions
*/
void UART_debug_Buffer(U8 length)
{
    U8 i;

    UART_Send("\r \n buffer: \r \n");

    for(i=0; (i <= length) && (i < UART_BUFFER_SIZE); i++)
    {
        UART_Tx(UART_buffer[i]);
        UART_Tx(';');
    }

    UART_newline();

    UART_Send("rx_pointer:");
    UART_Print_U8(rx_ptr);

    UART_newline();

    UART_Send("avail funct says:");
    UART_Print_U8(UART_available());
}



/*************************************************************************************
* USART Receive complete Interrupt
*************************************************************************************/
ISR(USART_RX_vect)
{
    /**
    this implements a circular buffer
    we can write new data to buffer as long as rx_ptr does not touch rd_ptr
    this will be when rx >= rd, as long as rx is not on the last element and rd on the first
    or when rx is 2 elements behind rd
    */
    if( ((rx_ptr >= rd_ptr) && (rx_ptr != UART_BUFFER_SIZE -1)) || ((rx_ptr == UART_BUFFER_SIZE -1) && (rd_ptr > 0)) || (rx_ptr < rd_ptr -1) )
    {
        /**
        save to buffer
        */
        UART_buffer[rx_ptr]= UDR0;
        rx_ptr++;

        if(rx_ptr == UART_BUFFER_SIZE)
        {
            rx_ptr =0;
        }
    }
}
