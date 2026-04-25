#ifndef __ADC_HPP
#define __ADC_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx.h"

#ifdef __cplusplus
}
#endif

class ADC_t {
public:
	uint16_t colourTemp = 0;
	uint16_t brightness = 0;

    void Init();
    uint16_t Measure(uint8_t pin_num);
    void MeasureAll();
};

#endif /* __ADC_HPP */
