#include "snake.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "../../BSP/TIM2/tim2.h"
#include "../../Utilities/timeout.h"

Snake snake;

Particle fruit;

dataMPU6050 mpu;


void initParticles()
{
	snake.length = 4;

	for(uint8_t i = 0; i < snake.length; i++)
	{
		snake.particles[i].x = snake.length - i;
		snake.particles[i].y = 4;
	}

	do
	{
		fruit.x = rand() % 32;
		fruit.y = rand() % 8;
	}
	while(isSomethingThere(fruit.x, fruit.y));
	return;
}

void initGame()
{

	initMPU6050();
	initMAX7219();
	initTim2();
	initParticles();

	// Testing daisy-chained MAX7219:

	/*
	 * testAllOn();
	 * testLine();
	 */

	return;
}

uint8_t isSomethingThere(uint8_t x, uint8_t y)
{
	for(int i = 0; i < snake.length; i++)
	{
		if(snake.particles[i].x == x && snake.particles[i].y == y)
		{
			return 1;
		}
	}

	return 0;
}


void moveParticles(Movement move)
{
	for(uint8_t i = snake.length - 1; i > 0; i--)
	{
		snake.particles[i] = snake.particles[i - 1];
	}

	switch(move)
	{
	case RIGHT:
		snake.particles[0].y++;
		break;
	case LEFT:
		snake.particles[0].y--;
		break;
	case UP:
		snake.particles[0].x++;
		break;
	case DOWN:
		snake.particles[0].x--;
		break;

	default: break;
	}

	if((snake.particles[0].x == fruit.x) && (snake.particles[0].y == fruit.y))
	{
		snake.length++;
		do
		{
			fruit.x = rand() % 32;
			fruit.y = rand() % 8;
		}
		while(isSomethingThere(fruit.x, fruit.y));
	}

	snake.particles[0].x %= DAISY_MATRIX_SIZE_X;
	snake.particles[0].y %= DAISY_MATRIX_SIZE_Y;

	return;
}

void renderParticles()
{
	clearMatrix();

	setPixel(fruit.x, fruit.y, 1);

	for(int i = 0; i < snake.length; i++)
	{
		setPixel(snake.particles[i].x, snake.particles[i].y, 1);
	}

	renderMAX7219();

	return;
}

void processGame(wiringMAX7219 wiring)
{
	static float rollFiltered = 0;
	static float pitchFiltered = 0;
	static Movement lastMove;

	if(timer2_flag)
	{
		timer2_flag = 0;

		getDataMPU6050(&mpu);

		rollFiltered = mpu.roll;
		pitchFiltered = mpu.pitch;

		Movement move = NONE;

		if(fabsf(rollFiltered) > fabsf(pitchFiltered))
		{
			if((rollFiltered > THRESHOLD) && (lastMove != RIGHT))
			{
				move = LEFT;
			}
			else if ((rollFiltered < -THRESHOLD) && (lastMove != LEFT))
			{
				move = RIGHT;
			}
		}
		else
		{
			if((pitchFiltered > THRESHOLD) && (lastMove != DOWN))
			{
				move = UP;
			}
			else if((pitchFiltered < -THRESHOLD) &&  (lastMove != UP))
			{
				move = DOWN;
			}
		}

		if(move != NONE)
		{
			lastMove = move;

			//renderParticles();		// 1 modul
		}

		moveParticles(lastMove);
		renderParticles();		// 4 daisy-chained moduls
	}
	return;
}
