#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include "stm32f10x.h"

#define TS_PORT USART1
#define DEBUG_PORT USART3

#define TS_RX_BUFFER_SIZE 24
//#define DEBUG_TX_BUFFER_SIZE 128

/**
When the serial buffer is filled to greater than this threshold value,
the serial processing operations will be performed more urgently in order to avoid it overflowing.
Serial buffer is 64 bytes long, so the threshold is set at half this as a reasonable figure
*/
#define SERIAL_BUFFER_THRESHOLD (TS_RX_BUFFER_SIZE / 2)


#define BUFFER_PULL_SUCCESS         0x00
#define BUFFER_PULL_ERROR_SEMAPHOR  0x01
#define BUFFER_PULL_ERROR_EMPTY     0x02
#define BUFFER_PUSH_ERROR_SEMAPHOR  0x03
#define BUFFER_PUSH_ERROR_FULL      0x04
#define BUFFER_PUSH_SUCCESS         0x00


typedef struct _serial_buffer_t {

    VU8 * const buffer;
    const uint32_t length;
    U32 head;
    U32 tail;
    U32 available;

} serial_buffer_t ;


//serial buffer functions
U32 serial_buffer_push(volatile serial_buffer_t * buffer, VU8 data_in);
U32 serial_buffer_available(volatile serial_buffer_t * buffer);
U32 serial_buffer_pull(volatile serial_buffer_t * buffer, VU8 * data_out);
void get_serial_buffer_access(volatile serial_buffer_t * buffer);
void free_serial_buffer_access(volatile serial_buffer_t * buffer);
void serial_buffer_reset(volatile serial_buffer_t * buffer);

//buffered serial communication
U32 UART_transmit(USART_TypeDef * Port, char data);
U32 UART_write(USART_TypeDef * Port, char * msg);

//init functions
void UART1_Init();
void UART3_Init();

//legacy direct hw access
void UART_Tx(USART_TypeDef * Port, char msg);
void UART_Send(USART_TypeDef * Port, char messg[] );


//buffered TS serial communication
U32 UART_getRX();
U32 UART_available();

//void UART_reset();


void UART_periodic();





#endif // UART_H_INCLUDED
