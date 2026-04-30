#include "project_config.h"
#include "adc.hpp"
#include "usart.hpp"
#include "stm32g0xx.h"


void ADC_t::Init() {

	// PA6 - Colour Temperature, PA7 - Brightness

	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	//Settings for GPIO PA6
    GPIOA->MODER |= GPIO_MODER_MODE6_0 | GPIO_MODER_MODE6_1;    // Analog mode enable
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD6;                          // No pull-up, pull-down

    //Settings for GPIO PA7
    GPIOA->MODER |= GPIO_MODER_MODE7_0 | GPIO_MODER_MODE7_1;    // Analog mode enable
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD7;                          // No pull-up, pull-down

    //Settings for ADC1
    RCC->APBENR2 |= RCC_APBENR2_ADCEN;     // ADC1 clock enable
    ADC1->CR &= ~ADC_CR_ADEN;              // Disable ADC
    while (ADC1->CR & ADC_CR_ADEN);        // Waiting ADC disable
    ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;      // Clear ADC clock mode
    ADC1->CFGR2 |= ADC_CFGR2_CKMODE_0;     // PCLK/2 (Synchronous clock mode) (ADC1 clock 8 Mhz)
    ADC1->CFGR1 &= ~ADC_CFGR1_ALIGN;       // Data right alignment
    ADC1->CFGR1 &= ~ADC_CFGR1_RES;         // Data resolution 12 bits
    ADC1->CFGR1 &= ~ADC_CFGR1_CONT;        // Single conversion mode
    ADC1->CFGR1 &= ~ADC_CFGR1_EXTEN;       // Hardware trigger detection disabled (conversions can be started by software)
    ADC1->CFGR1 &= ~ADC_CFGR1_WAIT;        // Wait conversion mode off
    ADC1->CFGR1 &= ~ADC_CFGR1_CHSELRMOD;   // Each bit of the ADC_CHSELR register enables an input

    if ((ADC1->CR & (ADC_CR_ADCAL | ADC_CR_ADSTP | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN)) && (ADC1->CFGR1 & ADC_CFGR1_AUTOFF)){
    	usart_printf("ADC is not ready to enable voltage regulator and calibrate\r");
    }
    else {
    	ADC1->CR |= ADC_CR_ADVREGEN;           // ADC voltage regulator enable
    	    for (volatile uint32_t i = 0; i < 500; i++) {} // ADC voltage regulator enabling (more 20 us)
    	    ADC1->CR |= ADC_CR_ADCAL;              // ADC calibrate
    	    while (ADC1->CR & ADC_CR_ADCAL);       // Waiting ADC calibration
    	    usart_printf("ADC calibrate success\r");
    }
    ADC1->CR |= ADC_CR_ADEN;               // ADC enable
    while (!(ADC1->ISR & ADC_ISR_ADRDY));  // Waiting when the ADC is ready to convert
    ADC1->ISR |= ADC_ISR_ADRDY;            // Clear flag ready
    usart_printf("The ADC is ready\r");

    ADC1->SMPR &= ~ADC_SMPR_SMP1;                       // Clear Sampling time selection 1
    ADC1->SMPR &= ~ADC_SMPR_SMP2;                       // Clear Sampling time selection 2
    ADC1->SMPR |= ADC_SMPR_SMP1_0 | ADC_SMPR_SMP1_2;    // 39.5 ADC clock cycles for SMP1
    ADC1->SMPR &= ~ADC_SMPR_SMPSEL6;                    // Sampling time of CHANNEL6 use the setting of SMP1 (for PA6)
    ADC1->SMPR &= ~ADC_SMPR_SMPSEL7;                    // Sampling time of CHANNEL7 use the setting of SMP1 (for PA7)

}

uint16_t ADC_t::Measure(uint8_t pin_num) {

	ADC1->CHSELR = (1U << pin_num);                     // Choosing channel

    ADC1->CR |= ADC_CR_ADSTART;                         // ADC start
    while (!(ADC1->ISR & ADC_ISR_EOC));                 // Waiting End of conversion flag

    uint16_t value = (uint16_t)ADC1->DR;                // Reading result

    ADC1->ISR |= ADC_ISR_EOC;                           // Clear  End of conversion flag

    return value;
}

void ADC_t::MeasureAll() {

	// Save local parameters between function call
	static uint16_t resultColourTemp = 0;
	static uint16_t resultBrightness = 0;
	static uint16_t lastColourTemp = 0;
	static uint16_t lastBrightness = 0;

	// Measure channels, filter 8 counts
	resultColourTemp = (lastColourTemp * 7 + this->Measure(6)) / 8;
	resultBrightness = (lastBrightness * 7 + this->Measure(7)) / 8;

	// Calculating the result (in percent)

	// ColourTemp
	if (resultColourTemp > 4075)
		this->colourTemp = 100;
	else if (resultColourTemp < 20)
		this->colourTemp = 0;
	else
		this->colourTemp = (resultColourTemp * 100U) / 4095U;
	lastColourTemp = resultColourTemp;

	// Brightness
	if (resultBrightness > 4075)
		this->brightness = 100;
	else if (resultBrightness < 20)
		this->brightness = 0;
	else
		this->brightness = (resultBrightness * 100U) / 4095U;
	lastBrightness = resultBrightness;

}
