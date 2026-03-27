#ifndef SPI_SPI_H_
#define SPI_SPI_H_

#define SCK			(1)		// PA1
#define CS    		(4)		// PA4
#define MOSI		(7)		// PA7

#define CS_LOW()	GPIOA->ODR &= ~GPIO_ODR_OD4;
#define CS_HIGH()	GPIOA->ODR |= GPIO_ODR_OD4;

#define BUFFER_SIZE			(2)
#define DAISY_CHAIN_NUMBER	(4)
#define DAISY_BUFFER_SIZE (BUFFER_SIZE * DAISY_CHAIN_NUMBER)

#define TIMEOUT_MS 100

void initSPI(void);
// write /w polling
void writeSPI(uint8_t address, uint8_t data);
// write /w interrupt
void writeSPI_Interrupt(uint8_t address, uint8_t data);
// write /w polling & daisy chained SPI moduls
void writeSPIDaisy(uint8_t address, uint8_t *dataArray);
// write /w interrupt & daisy chained SPI moduls
void writeSPIDaisy_Interrupt(uint8_t address, uint8_t *dataArray);

#endif
