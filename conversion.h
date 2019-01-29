#ifndef CONVERSION_H_INCLUDED
#define CONVERSION_H_INCLUDED

#include "stm32f10x.h"
#include "stm32_libs/boctok/boctok_types.h"

#define PAD 0xFFFFFFF
#define NO_PAD 0

typedef enum {

    TYPE_U8,
    TYPE_U16,
    TYPE_U32,

    TYPE_S8,
    TYPE_S16,
    TYPE_S32

} conversion_int_t ;

void UART_Print_S(USART_TypeDef * Port, S32 value, conversion_int_t inttype, U32 padding);
void UART_Print_U(USART_TypeDef * Port, U32 value, conversion_int_t inttype, U32 padding);

void CV_U8Char( U8 ,  char *);
void CV_U16Char( U16,  char *,  U8,  U8);
void UART_Print_U8(U8 value);
void CV_S8Char( S8 ,  char *);
void UART_Print_S8(S8 value);
void UART_Print_U16(U16 value);
void UART1_Print_U32(U32 value);

void UART_Print_U8Hex(USART_TypeDef * Port, U8 value);
void UART_Print_U8Hex_new(USART_TypeDef * Port, U8 value);

#endif // CONVERSION_H_INCLUDED
