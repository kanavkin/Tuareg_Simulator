#ifndef STM32_GPIO_H_INCLUDED
#define STM32_GPIO_H_INCLUDED

/**
how GPIO on STM32 looks like:
- GPIO ports A..G
- GPIO connected to APB2 bus
- 16 pins on each port
- register accessed in word mode (32bit)

- pins 0..7  controlled by GPIOx_CRL
- pins 8..15 controlled by GPIOx_CRH




*/

/**
remember to enable clock
RCC->APB2ENR |= RCC_APB2Periph_GPIOx;
or disable
RCC->APB1ENR &= ~RCC_APB2Periph_GPIOx;
*/

#define GPIO_IN_PUD         0x08
#define GPIO_IN_FLOAT       0x04
#define GPIO_ANALOG         0x00

#define GPIO_OUT_PP_2MHZ    0x02
#define GPIO_OUT_OD_2MHZ    0x06
#define GPIO_AF_PP_2MHZ     0x0A
#define GPIO_AF_OD_2MHZ     0x0E

#define GPIO_OUT_PP_10MHZ   0x01
#define GPIO_OUT_OD_10MHZ   0x05
#define GPIO_AF_PP_10MHZ    0x09
#define GPIO_AF_OD_10MHZ    0x0D

#define GPIO_OUT_PP_50MHz   0x03
#define GPIO_OUT_OD_50MHz   0x07
#define GPIO_AF_PP_50MHZ    0x0B
#define GPIO_AF_OD_50MHZ    0x0F


/**
AFIO EXTI mapping



*/
#define EXTI_MAP_GPIOA  0x00
#define EXTI_MAP_GPIOB  0x01
#define EXTI_MAP_GPIOC  0x02
#define EXTI_MAP_GPIOD  0x03
#define EXTI_MAP_GPIOE  0x04
#define EXTI_MAP_GPIOF  0x05
#define EXTI_MAP_GPIOG  0x06





void GPIO_configure(GPIO_TypeDef * port, uint32_t pin, uint32_t setup);
void AFIO_map_EXTI(uint32_t line, uint32_t port);


#endif // STM32_GPIO_H_INCLUDED
