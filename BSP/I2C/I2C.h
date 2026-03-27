#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "../../BSP/TIM2/tim2.h"

#define SCL			(9)
#define SDA			(10)

#define TIMEOUT_MS 100

void initI2C(void);
void writeRegI2C(uint8_t deviceAddress, uint8_t regAddress, uint8_t data);
void readRegI2C(uint8_t deviceAddress, uint8_t regAddress, uint8_t *buffer, uint8_t len);

#endif
