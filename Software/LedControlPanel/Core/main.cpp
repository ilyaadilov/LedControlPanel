#include "project_config.h"
#include "usart.hpp"
#include "stm32g0xx.h"


int main(void){

	// Start with HSI=16MHz
	//RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	//GPIOB->CRL &= ~GPIO_CRL_CNF2;
	//GPIOB->CRL |= GPIO_CRL_MODE2_0;
	//GPIOB->ODR |= GPIO_ODR_ODR2;

#ifdef USE_USART_LOGGING
	USART2_Init();
	usart_printf("SystemCoreClock = %u Hz\r\n", SystemCoreClock);
	usart_printf("Digit = %d, Udigit = %u, String = %s, Hex = %X\r\n", -1, 2, "Hello", 0x20000000);
#endif


	while (1)
	{

	}
}
