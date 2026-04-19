#include "project_config.h"
#include "usart.hpp"
#include "stm32g0xx.h"


void USART2_Init(void){

	//PA2 - TX, PA3 - RX, speed - 115200, Full duplex

	RCC->APBENR1 |= RCC_APBENR1_USART2EN;
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	GPIOA->MODER &= ~GPIO_MODER_MODE2;              // Clear MODE for PA2
	GPIOA->MODER |= GPIO_MODER_MODE2_1;             // Alternate function mode
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;              // Output push-pull
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED2;        // Clear OSPEEDR for PA2
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_0;       // Medium speed
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2);            // Clear PUPDR
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2);           // Clear Alternate function for PA2
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_0;            // AF1 (USART2_TX)

	GPIOA->MODER &= ~GPIO_MODER_MODE3;              // Clear MODE for PA3
	GPIOA->MODER |= GPIO_MODER_MODE3_1;             // Alternate function mode
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED3;        // Clear OSPEEDR for PA3
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED3_0;       // Medium speed
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;              // Clear PUPDR
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;             // pull-up
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL3);           // Clear Alternate function for PA3
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_0;            // AF1 (USART2_RX)

	USART2->BRR = 16000000U / 115200U;              // 0x08A for FCLK = 16 MHz and baudrate = 115200

	USART2->CR1 |= USART_CR1_TE;          // Transmitter Enable
	USART2->CR1 |= USART_CR1_UE;          // Enable USART

}

void USART2_Send_Char(char chr){

	while(!(USART2->ISR & USART_ISR_TXE_TXFNF));
	USART2->TDR = chr;
	//while (!(USART2->ISR & USART_ISR_TC)) {}
}

void USART2_Send_Str(const char* str){

	uint8_t i = 0;
	while(str[i]) USART2_Send_Char(str[i++]);
}
