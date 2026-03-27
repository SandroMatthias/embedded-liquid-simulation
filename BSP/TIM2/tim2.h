#ifndef INC_TIM2_H_
#define INC_TIM2_H_

#define CLK_VALUE	64000000
#define PSC_VALUE	6400//8000//32000
#define ARR_VALUE 	166//62

//	Calculates the sampling rate
#define DT	1.0f //1.0 / ((CLK_VALUE / PSC_VALUE * 1.0) / ARR_VALUE)

extern volatile uint8_t timer2_flag;

void initTim2(void);

#endif
