#ifndef INC_SNAKE_H_
#define INC_SNAKE_H_

#include "../../Devices/MPU6050/MPU6050.h"
#include "../../Devices/MAX7219/MAX7219.h"

#define MAX_LENGTH	16

typedef struct
{
	Particle particles[MAX_LENGTH];
	uint8_t length;
} Snake;

void initGame();
void initParticles();
void moveParticles(Movement move);
uint8_t isSomethingThere(uint8_t x, uint8_t y);
void renderParticles();

void processGame(wiringMAX7219 wiring);

#endif
