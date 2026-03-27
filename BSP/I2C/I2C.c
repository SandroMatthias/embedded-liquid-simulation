#include <stdint.h>

#include "stm32g0xx.h"
#include "I2C.h"
#include "../../Utilities/timeout.h"

static void initClockI2C(void)
{
	// ENABLE IO CLOCK
	RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;

	// ENABLE PERIPHERAL CLOCK
	RCC->APBENR1 |= RCC_APBENR1_I2C1EN;
	return;
}

static void configPortI2C(void)
{
	/* RESET & SET MODE */
	GPIOA->MODER &= ~(0b11 << (SCL * 2)|
					  0b11 << (SDA * 2)
					  );

	// SCL & SDA - AF
	GPIOA->MODER |= 0b10 << (SCL * 2)|
					0b10 << (SDA * 2);

	/* OUTPUT TYPE */
	GPIOA->OTYPER |= 0b01 << SCL |
					 0b01 << SDA;

	/* OUTPUT SPEED - high speed*/
	GPIOA->OSPEEDR |= 0b11 << (SCL * 2)|
			  	  	  0b11 << (SDA * 2);

	/* PULL-UP/PULL-DOWN - pull-up*/
	GPIOA->PUPDR &= ~(0b11 << (SCL * 2)|
					  0b11 << (SDA * 2)
					  );

	GPIOA->PUPDR |= 0b01 << (SCL * 2)|
					0b01 << (SDA * 2);

	/* AF MODE - AF6 */
	GPIOA->AFR[1] &= ~(0xF << ((SCL - 8) * 4)|
					   0XF << ((SDA - 8) * 4)
					   );

	GPIOA->AFR[1] |=  (0x6 << ((SCL - 8) * 4)|
					   0x6 << ((SDA - 8) * 4)
					   );
	return;
}

static void configI2C(void)
{
	/* DISABLE I2C */
	I2C1->CR1 &= ~I2C_CR1_PE;

	/* TIMING -> 64MHz to 100kHz */
	I2C1->TIMINGR = 0x00B03FDB;

	/* ENABLE I2C */
	I2C1->CR1 |= I2C_CR1_PE;

	return;
}

void initI2C(void)
{
	initClockI2C();
	configPortI2C();
	configI2C();
	return;
}

void writeRegI2C(uint8_t deviceAddress, uint8_t regAddress, uint8_t data)
{
	// Check if I2C or bus is busy
	if(!flagTimeout(&I2C1->ISR, I2C_ISR_BUSY, TIMEOUT_MS, 0));

	// Configure the transfer
	I2C1->CR2 =((deviceAddress << I2C_CR2_SADD_Pos)|		// Which 7-bit slave address to talk to
				(2 << I2C_CR2_NBYTES_Pos)|					// Tells the peripheral that 2 bytes will be sent (address + data)
				(I2C_CR2_START));							// Generates the start condition

	// Transmit Interrupt Status set -> if 1 == TXDR register is empty, ready to next byte
	if(!flagTimeout(&I2C1->ISR, I2C_ISR_TXIS, TIMEOUT_MS, 1));
	// Hardware shifts it out to the bus -> MPU will see the register address
    I2C1->TXDR = regAddress;

    if(!flagTimeout(&I2C1->ISR, I2C_ISR_TXIS, TIMEOUT_MS, 1));
    I2C1->TXDR = data;

    // Transfer Complete flag -> all bytes transfered + ack without generating STOP
    if(!flagTimeout(&I2C1->ISR, I2C_ISR_TC, TIMEOUT_MS, 1));

    I2C1->CR2 |= I2C_CR2_STOP;

	return;
}

void readRegI2C(uint8_t deviceAddress, uint8_t regAddress, uint8_t *buffer, uint8_t len)
{
	// Check if I2C or bus is busy
    if(!flagTimeout(&I2C1->ISR, I2C_ISR_BUSY, TIMEOUT_MS, 0));

    // Configure the transfer
    I2C1->CR2 = (deviceAddress << I2C_CR2_SADD_Pos) |
                (1 << I2C_CR2_NBYTES_Pos) |
                I2C_CR2_START;

    // Transmit Interrupt Status set
    if(!flagTimeout(&I2C1->ISR, I2C_ISR_TXIS, TIMEOUT_MS, 1));

    I2C1->TXDR = regAddress;

    // Transfer Complete flag
    if(!flagTimeout(&I2C1->ISR, I2C_ISR_TC, TIMEOUT_MS, 1));

    I2C1->CR2 = (deviceAddress << I2C_CR2_SADD_Pos) |	// Address
                (len << I2C_CR2_NBYTES_Pos) |			// Number of bytes
                I2C_CR2_RD_WRN |						// Set READ mode
                I2C_CR2_START |
                I2C_CR2_AUTOEND;						// Automatically sends STOP

    for(int i = 0; i < len; i++)
    {
        // Wait until Receive Data Register Not Empty
        if(!flagTimeout(&I2C1->ISR, I2C_ISR_RXNE, TIMEOUT_MS, 1));
        buffer[i] = I2C1->RXDR;							// Read the received byte
    }

    // Stop Detection Flag
    if(!flagTimeout(&I2C1->ISR, I2C_ISR_STOPF, TIMEOUT_MS, 1));
    I2C1->ICR |= I2C_ICR_STOPCF;
    return;
}

