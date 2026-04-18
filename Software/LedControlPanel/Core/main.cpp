#include "project_config.h"
#include <cstring>
#include "usart.hpp"
#include "stm32g0xx.h"

int main(void){

	// Start with HSI=16MHz
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE4;              // Clear MODE for PA4
	GPIOA->MODER |= GPIO_MODER_MODE4_0;             // Output mode
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;              // 00b → No Pull

#ifdef USE_USART_LOGGING
	USART2_Init();
	usart_printf("SystemCoreClock = %u Hz\r\n", SystemCoreClock);
	usart_printf("Digit = %d, Udigit = %u, String = %s, Hex = %X\r\n", -1, 2, "Hello", 0x20000000);
#endif

	while (1)
	{
		GPIOA->ODR ^= GPIO_ODR_OD4;
		for(int i = 0; i<1000000; i++);
	}
}
