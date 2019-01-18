#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED


#define FOSC 16000000
#define BAUD 115200
#define MYUBRR FOSC/16/BAUD-1

void UART_Init();

//send string (0 terminated)
void UART_Send(char [] );

//send byte
void UART_Tx( char );

#endif // UART_H_INCLUDED
