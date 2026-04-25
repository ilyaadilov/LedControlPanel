#include "project_config.h"
#include <cstring>
#include "usart.hpp"
#include "adc.hpp"
#include "tim.hpp"
#include "stm32g0xx.h"

void Timeout_ms_Init();
void timeout_ms(uint32_t timeout_ms);

ADC_t Adc;
TIM_t Tim2;

int main(void){

	// Start with HSI=16MHz
	Timeout_ms_Init();
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE4;              // Clear MODE for PA4
	GPIOA->MODER |= GPIO_MODER_MODE4_0;             // Output mode
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;              // No pull-up\pull-down

#ifdef USE_USART_LOGGING
	USART2_Init();
	usart_printf("SystemCoreClock = %u Hz\r\n", SystemCoreClock);
	usart_printf("Digit = %d, Udigit = %u, String = %s, Hex = %X\r\n", -1, 2, "Hello", 0x20000000);
#endif

	Adc.Init();
	uint16_t resultColourTemp = 0;
	uint16_t resultBrightness = 0;

	Tim2.Init();

	while (1)
	{
		resultColourTemp = Adc.MeasurePercent(6);
		resultBrightness = Adc.MeasurePercent(7);
		usart_printf("ColourTemperature = %u\r\n", resultColourTemp);
		usart_printf("Brightness = %u\r\n", resultBrightness);
		Tim2.SetDuty(1, resultColourTemp);
		Tim2.SetDuty(2, resultBrightness);
		GPIOA->ODR ^= GPIO_ODR_OD4;
		timeout_ms(1000);
	}
}

void Timeout_ms_Init() {

    SysTick->LOAD = 15999U;          // Autoreload value (16 Mhz / 1000)
    SysTick->VAL  = 0U;              // Reset value

    // CLKSOURCE=1 (source = HCLK), TICKINT=0 (interrupt disable), ENABLE=1
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

}

void timeout_ms(uint32_t timeout_ms) {

    while (timeout_ms > 0U) {
        SysTick->VAL = 0U; // Reset counter

        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // Waiting COUNTFLAG and reset it

        timeout_ms--;
    }

}
