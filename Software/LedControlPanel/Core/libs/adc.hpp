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

    void Init();

    uint16_t Measure(uint8_t pin_num);
};

#endif /* __ADC_HPP */
