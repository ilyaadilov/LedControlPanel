#include "project_config.h"
#include "tim.hpp"
#include "usart.hpp"
#include "stm32g0xx.h"


void TIM_t::Init() {

	// PA0 - Cold Led Strip , PA1 - Warm Led Strip

	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	//Settings for GPIO PA0
	GPIOA->MODER &= ~GPIO_MODER_MODE0;              // Clear MODE
	GPIOA->MODER |= GPIO_MODER_MODE0_1;             // Alternate function mode
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT0;              // Output push-pull
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED0;        // Clear OSPEEDR, Low speed
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0);            // Clear PUPDR
	//GPIOA->PUPDR |= GPIO_PUPDR_PUPD0_1;             // Pull-down
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL0);           // Clear Alternate function
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_1;            // AF2 (IM2_CH1_ETR)

    //Settings for GPIO PA1
	GPIOA->MODER &= ~GPIO_MODER_MODE1;              // Clear MODE
	GPIOA->MODER |= GPIO_MODER_MODE1_1;             // Alternate function mode
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;              // Output push-pull
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED1;        // Clear OSPEEDR, Low speed
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD1);            // Clear PUPDR
	//GPIOA->PUPDR |= GPIO_PUPDR_PUPD1_1;             // Pull-down
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL1);           // Clear Alternate function
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL1_1;            // AF2 (IM2_CH2)

    //Settings for TIM2
    RCC->APBENR1 |= RCC_APBENR1_TIM2EN;              // TIM2 clock enable
    TIM2->CR1 &= ~TIM_CR1_CEN;                       // Disable TIM2
    TIM2->PSC = 0;                                   // Prescaler: 16 MHz / (0+1) = 16 MHz for clock TIM2
    TIM2->ARR = 639;                                 // Auto-reload value: 1 MHz / (639+1) ≈ 25 kHz

    TIM2->CCMR1 = 0;                                     // Clear CCMR1,  CC1 and CC1 channels is configured as output (CC1S = 00)
    //for channel 1 (PA0)
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;  // Output compare PMW mode 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;                      // Output Compare 1 Preload enable
    TIM2->CCER |= TIM_CCER_CC1E;                         // OC1 signal is output on the corresponding output pin
    //for channel 2 (PA1)
    TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;  // Output compare PMW mode 1
    TIM2->CCMR1 |= TIM_CCMR1_OC2PE;                      // Output Compare 2 Preload enable
    TIM2->CCER |= TIM_CCER_CC2E;                         // OC2 signal is output on the corresponding output pin

    TIM2->CR1 |= TIM_CR1_ARPE;                        // Auto-reload preload enable
    TIM2->CR1 |= TIM_CR1_CEN;                         // TIM2 enable

    usart_printf("TIM2 starts PWM\r");

}

void TIM_t::SetDuty(uint8_t channel, uint32_t percent) {

    if (percent > 100) percent = 100;

    uint32_t ccr = (percent * 640U) / 100U;

    if (channel == 1)      TIM2->CCR1 = ccr;
    else if (channel == 2) TIM2->CCR2 = ccr;

}
