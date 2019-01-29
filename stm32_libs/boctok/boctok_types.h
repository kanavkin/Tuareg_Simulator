#ifndef BOCTOK_TYPES_H_INCLUDED
#define BOCTOK_TYPES_H_INCLUDED


#include <stdlib.h>
#include <stdint.h>


// Standard data types
typedef uint8_t     U8;
typedef uint16_t    U16;
typedef uint32_t    U32;
typedef uint64_t    U64;

typedef int8_t     S8;
typedef int16_t    S16;
typedef int32_t    S32;

typedef volatile uint8_t     VU8;
typedef volatile uint16_t    VU16;
typedef volatile int8_t     VS8;
typedef volatile int16_t    VS16;
typedef volatile uint32_t    VU32;
typedef volatile int32_t    VS32;


#define FALSE 0
#define TRUE 0x01


#endif // TYPES_H_INCLUDED
