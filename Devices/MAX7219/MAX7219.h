#ifndef MAX7219_MAX7219_H_
#define MAX7219_MAX7219_H_

#include <stdint.h>
#include "../../BSP/SPI/SPI.h"

#define MATRIX_SIZE 		8

#define DAISY_MATRIX_SIZE_X	(DAISY_CHAIN_NUMBER * 8)
#define DAISY_MATRIX_SIZE_Y	8

typedef enum {NOT_DAISY, DAISY} wiringMAX7219;

typedef struct
{
	uint8_t x;
	uint8_t y;
	float vx;
	float vy;
} Particle;

typedef struct
{
	float x;
	float y;
	float vx;
	float vy;
	float p;
} liquidParticle;

void setPixel(uint8_t x, uint8_t y, uint8_t state);
void clearMatrix(void);

void initMAX7219(void);
void clearMAX7219(void);
void renderMAX7219(void);

void counter();

void testAllOn();
void testLine();

#endif
