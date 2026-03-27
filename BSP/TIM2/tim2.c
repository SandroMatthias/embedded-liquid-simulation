#include <stdint.h>

#include "tim2.h"
#include "stm32g0xx.h"

volatile uint8_t timer2_flag;

void initTim2(void)
{
	timer2_flag = 0;

    RCC->APBENR1 |= RCC_APBENR1_TIM2EN;

    // CLK div: 64Mhz/PSC -> allowing to count slower
    TIM2->PSC = PSC_VALUE - 1;

    // Auto-reload clk/arr = second -> this is the MAX count value
    TIM2->ARR = ARR_VALUE - 1;

    // Interrupt enable
    TIM2->DIER |= TIM_DIER_UIE;

    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN;

	return;
}

void TIM2_IRQHandler(void)
{
	// Update interrupt flag -> signals when counter hits the ARR value
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;

        timer2_flag = 1;
    }
}

