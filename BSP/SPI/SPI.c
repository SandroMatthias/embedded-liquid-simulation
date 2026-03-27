#include <stdint.h>

#include "stm32g0xx.h"
#include "SPI.h"
#include "../../Utilities/timeout.h"

static uint8_t TxBuffer[BUFFER_SIZE];
static uint8_t TxBufferDaisy[DAISY_BUFFER_SIZE];
static uint8_t idx;
static uint8_t TxSize;

static void initClockSPI(void)
{
	// ENABLE IO CLOCK
	RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;

	// ENABLE PERIPHERAL CLOCK
	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;
	return;
}

static void configPortSPI(void)
{
	/* RESET & SET MODE */
	GPIOA->MODER &= ~(0b11 << (SCK * 2)|
					  0b11 << (CS * 2) |
					  0b11 << (MOSI * 2)
					  );

	// SCK & MOSI - AF
	GPIOA->MODER |= 0b10 << (SCK * 2)|
					0b10 << (MOSI * 2);

	// CS - OUTPUT
	GPIOA->MODER |= 0b01 << (CS * 2);

	/* AF MODE */
	GPIOA->AFR[0] &= ~(0xF << (SCK * 4) |
					   0XF << (MOSI * 4));

	GPIOA->AFR[0] |=  (0x0 << (SCK * 4) |
					   0x0 << (MOSI * 4));

	/* PULL HIGH THE CS LINE */
	CS_HIGH()

	return;
}

static void configSPI(void)
{
	SPI1->CR1 = 0;
	SPI1->CR2 = 0;

	/* SET & RESET CONTROL REGISTERS */
	SPI1->CR1 |= (0b010 << SPI_CR1_BR_Pos |			// BAUDE RATE, prescaler 64 (64MHz/64) - 250kHz
				  SPI_CR1_MSTR|						// Master configuration
				  SPI_CR1_SSM |						// Software slave management enabled
				  SPI_CR1_SSI |						// Internal slave select
				  SPI_CR1_BIDIMODE |				// 1-line direction (MAX7219 receives & don't transmits)
				  SPI_CR1_BIDIOE					// Enable transmit-only mode
				  );

	SPI1->CR1 &= ~(SPI_CR1_CPOL |					// SPI MODE 0 -> POLARITY & PHASE = 0
	  	  	  	   SPI_CR1_CPHA |
				   SPI_CR1_LSBFIRST	|				// MSB first
				   SPI_CR1_RXONLY					// this bit needs to be cleared because 1-line BIDIMODE
				   );

	SPI1->CR2 |= ((0b0111 << SPI_CR2_DS_Pos) |		// Data size = 8 bit
				  SPI_CR2_FRXTH						// The RXNE flag triggers when the FIFO reaches at least 8 bits of data
				  );

	/* AFTER CONFIG -> ENABLE SPI */
	SPI1->CR1 |= SPI_CR1_SPE;

	return;
}

void initSPI(void)
{
	initClockSPI();
	configPortSPI();
	configSPI();

	NVIC_SetPriority(SPI1_IRQn, 1);		// lower number higher prio
	NVIC_EnableIRQ(SPI1_IRQn);			// IRQ - iterrupt vector index

	return;
}

void writeSPI(uint8_t address, uint8_t data)
{
	/* PULL LOW THE CS LINE TO START COMMUNICATION */
	CS_LOW()

	// Transmit buffer empty -> stage data before it is
	// sent via the SPI shift register onto the MOSI line
	if(!flagTimeout(&SPI1->SR, SPI_SR_TXE, TIMEOUT_MS, 1)) return;

	// DR (data register)   -> 32 bit register
	// &SPI1->DR 			-> memory address of the register (type uint32_t *)
	// (volatile uint8_t*)  -> cast to a pointer to 8-bit + no optimmizations + HW access
	// *((...)) 			-> dereference the pointer
	// With this: writes a single byte to data register
	*((volatile uint8_t*)&SPI1->DR) = address;

	if(!flagTimeout(&SPI1->SR, SPI_SR_TXE, TIMEOUT_MS, 1)) return;
	*((volatile uint8_t*)&SPI1->DR) = data;

	if(!flagTimeout(&SPI1->SR, SPI_SR_BSY, TIMEOUT_MS, 0));

	/* PULL HIGH THE CS LINE TO STOP COMMUNICATION */
	CS_HIGH()

	return;
}

void writeSPIDaisy(uint8_t address, uint8_t *dataArray)
{
	/* PULL LOW THE CS LINE TO START COMMUNICATION */
	CS_LOW()

	for(int i = DAISY_CHAIN_NUMBER; i >= 0; i--)
	{
		// Send address
		if(!flagTimeout(&SPI1->SR, SPI_SR_TXE, TIMEOUT_MS, 1)) return;
		*((volatile uint8_t*)&SPI1->DR) = address;

		// Send data for this module
		if(!flagTimeout(&SPI1->SR, SPI_SR_TXE, TIMEOUT_MS, 1)) return;
		*((volatile uint8_t*)&SPI1->DR) = dataArray[i];
	}

	if(!flagTimeout(&SPI1->SR, SPI_SR_BSY, TIMEOUT_MS, 0)) return;

	CS_HIGH()

	return;
}

void writeSPI_Interrupt(uint8_t address, uint8_t data)
{
	TxBuffer[0] = address;
	TxBuffer[1] = data;

	idx = 0;
	TxSize = 2;

	CS_LOW()

	SPI1->CR2 |= SPI_CR2_TXEIE;

	/*
	 * Enable interrupt TXEIE -> Tx buffer empty interrupt enable
	 *
	 * Hardware generates an interrupt request (IRQ) when -> TXEIE = 1
	 * Meaning: the transmit register is empty -> ready for new byte
	 *
	 * NVIC calls the ISR (SPI1_IRQHandler):
	 * Write next byte from buffer to DR -> hardware shifts it
	 * Repeat the ISR call until all bytes sent
	 *
	 * After last byte:
	 * Disable interrupt, pull CS HIGH
	 *
	 */

	return;
}

void writeSPIDaisy_Interrupt(uint8_t address, uint8_t *dataArray)
{
	idx = 0;
	TxSize = DAISY_BUFFER_SIZE;
    int k = 0;

	for(int i = DAISY_CHAIN_NUMBER - 1; i >= 0; i--)
	{
		TxBufferDaisy[k++] = address;
		TxBufferDaisy[k++] = dataArray[i];
	}

	CS_LOW()

	SPI1->CR2 |= SPI_CR2_TXEIE;

	return;
}

void SPI1_IRQHandler(void)
{
	// TX buffer empty -> ready to accept the next byte
	if(SPI1->SR & SPI_SR_TXE)
	{
		if(idx < TxSize)
		{
			// Pusher 1 byte to FIFO
			// TXE becomes 0 -> after data shifted out -> interrupt fires again
			*((volatile uint8_t *)&SPI1->DR) = TxBufferDaisy[idx++];
		}
		else
		{
			// After all data is transmitted -> disable interrupt
			SPI1->CR2 &= ~SPI_CR2_TXEIE;

			if(!flagTimeout(&SPI1->SR, SPI_SR_BSY, TIMEOUT_MS, 0)) return;

			CS_HIGH()

			idx = 0;
			TxSize = 0;
		}
	}
	return;
}
