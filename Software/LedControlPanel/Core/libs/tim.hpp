#ifndef __TIM_HPP
#define __TIM_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx.h"

#ifdef __cplusplus
}
#endif

class TIM_t {
public:

    void Init();

    void SetDuty(uint8_t channel, uint32_t percent);
};

#endif /* __TIM_HPP */
