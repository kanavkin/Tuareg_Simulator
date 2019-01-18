#include "stm32f10x.h"


void GPIO_configure(GPIO_TypeDef * port, uint32_t pin, uint32_t setup)
{
    if(pin < 8)
    {
        /**
        CRL
        pins 0..7
        clear old values first, even after reset there is 0x04!
        */
        port->CRL &= (uint32_t) ~(0x0F << (pin * 4));
        port->CRL |= (uint32_t)(setup << (pin * 4));
    }
    else
    {
        /**
        CRH
        pins 8..15
        clear old values first, even after reset there is 0x04!
        */
        port->CRH &= (uint32_t) ~(0x0F << ((pin - 8) * 4));
        port->CRH |= (uint32_t)(setup << ((pin - 8) * 4));
    }
}


void AFIO_map_EXTI(uint32_t line, uint32_t port)
{

    if(line < 4)
    {
        AFIO->EXTICR[0] &= (uint32_t) ~(0x0F << (line * 4));
        AFIO->EXTICR[0] |= (uint32_t)(port << (line * 4));
    }
    else if(line < 8)
    {
        AFIO->EXTICR[1] &= (uint32_t) ~(0x0F << ((line -4) * 4));
        AFIO->EXTICR[1] |= (uint32_t)(port << ((line -4) * 4));
    }
    else if(line < 12)
    {
        AFIO->EXTICR[2] &= (uint32_t) ~(0x0F << ((line -8) * 4));
        AFIO->EXTICR[2] |= (uint32_t)(port << ((line -8) * 4));
    }
    else if(line < 16)
    {
        AFIO->EXTICR[3] &= (uint32_t) ~(0x0F << ((line -12) * 4));
        AFIO->EXTICR[3] |= (uint32_t)(port << ((line -12) * 4));
    }

}
