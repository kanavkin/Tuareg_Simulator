#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#define UART_BUFFER_SIZE 64

/**
When the serial buffer is filled to greater than this threshold value,
the serial processing operations will be performed more urgently in order to avoid it overflowing.
Serial buffer is 64 bytes long, so the threshold is set at half this as a reasonable figure
*/
#define SERIAL_BUFFER_THRESHOLD 32

void UART_Init();

//send string (0 terminated)
void UART_Send(char [] );
void UART_Send_P(const char *data);

//send byte
void UART_Tx( char );

void UART_newline();

void UART_reset();
void UART_nolisten();

void UART_debug_Buffer(U8 length);



U8 UART_available();
U8 UART_getRX();

#endif // UART_H_INCLUDED
