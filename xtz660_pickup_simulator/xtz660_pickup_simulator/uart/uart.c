#include <avr/io.h>
#include <avr/interrupt.h>

#include "../types.h"
#include "uart.h"

uint8_t ubrr= MYUBRR;


void UART_Init()
{
    //baud rate 115200
	UBRR0H= 0;
	UBRR0L= 16;

	//double speed
	UCSR0A= (1<< U2X0);

	//uart module enabled
	UCSR0B= (1<< RXEN0) | (1<< TXEN0);

	//no parity, 1 stop bit, 8 data bits
	UCSR0C= (1<< UCSZ01) | (1<< UCSZ00);
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

