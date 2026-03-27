#include "timeout.h"

volatile uint32_t systemTick = 0;

// SysTick_Handler in stm32g0xx_it.c is commented out!
void SysTick_Handler(void)
{
	systemTick++;
	return;
}

void initSysTick(void)
{
	//	64MHz / 1000 = 64kHz	--> 1ms
	SysTick->LOAD = (SystemCoreClock / 1000) - 1;		// SystemCoreClock - MCU clock frequency (64MHz)
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |		// Use core system clock
            		SysTick_CTRL_TICKINT_Msk |			// Enable interrupt
					SysTick_CTRL_ENABLE_Msk;
	return;
}

uint32_t getSysTick()
{
	return systemTick;
}

uint8_t flagTimeout(volatile uint32_t *regAddress, uint32_t regFlag, uint32_t msTimeout, uint8_t targetState)
{
	uint32_t startTime = getSysTick();

	while(1)
	{
		uint8_t flagState = ((*regAddress & regFlag) != 0); // is this bit set?

		 if(targetState == flagState) return 1;

		 if((getSysTick() - startTime) >= msTimeout) return 0;
	}

	return 1;
}

void timeOut(uint32_t msTimeout)
{
	uint32_t startTime = getSysTick();
	while(1)
	{

		 if((getSysTick() - startTime) >= msTimeout) return;
	}
	return;
}
