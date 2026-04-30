#include "project_config.h"
#include <cstring>
#include "usart.hpp"
#include "adc.hpp"
#include "tim.hpp"
#include "stm32g0xx.h"

#define LED_COLD 1         // Channel 1 TIM2
#define LED_HOT  2         // Channel 2 TIM2

void Timeout_ms_Init();
void timeout_ms(uint32_t timeout_ms);

ADC_t Adc;
TIM_t Tim2;


int main(void){

	// Start with HSI=16MHz
	Timeout_ms_Init();

	USART2_Init();
	usart_printf("SystemCoreClock = %u Hz\r\n", SystemCoreClock);

	Adc.Init();
	Tim2.Init();

	USART2_DeInit();

	while (1)
	{
		Adc.MeasureAll();

		// ColourTemperature and Brightness changing
		if (Adc.colourTemp >= 50){
			Tim2.SetDuty(LED_COLD, Adc.brightness);
			Tim2.SetDuty(LED_HOT, ((100 - 2 * (Adc.colourTemp - 50)) * Adc.brightness) / 100);
		}
		else{
			Tim2.SetDuty(LED_HOT, Adc.brightness);
			Tim2.SetDuty(LED_COLD, (2 * Adc.colourTemp * Adc.brightness) / 100);
		}

		timeout_ms(10);
	}

}


// === Support functions ===

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
