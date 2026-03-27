#ifndef INC_TIMEOUT_H_
#define INC_TIMEOUT_H_

#include "stm32g0xx.h"
#include <stdint.h>

void initSysTick(void);

uint32_t getSysTick();

uint8_t flagTimeout(volatile uint32_t *reg, uint32_t flag, uint32_t msTimeout, uint8_t waitForSet);
void timeOut(uint32_t msTimeout);

#endif
