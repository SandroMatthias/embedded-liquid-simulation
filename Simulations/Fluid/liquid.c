#include "liquid.h"
#include "../../Devices/MPU6050/MPU6050.h"
#include "../../Devices/MAX7219/MAX7219.h"
#include "../../Utilities/timeout.h"
#include "../../BSP/TIM2/tim2.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

liquidParticle liquidParticles[LIQUID_PARTICLES_NUMBER];

dataMPU6050 mpu_liquid;

Vector gravity;
Vector distanceVector;

float dt = DT;

/*
 * @brief:					When creating a liquid particle, there is a chance
 * 							that multiple particles may be created at the same coordinate.
 *
 * @retval:  				None
 *
 * @note:					Clearin, setting & rendering
 */
static uint8_t isParticleThere(uint8_t x, uint8_t y, uint8_t createdParticles)
{
	for(int i = 0; i < createdParticles; i++)
	{
		if(liquidParticles[i].x == x && liquidParticles[i].y == y)
		{
			return 1;
		}
	}

	return 0;
}

/*
 * @brief:					Initializes (creates) the particles randomly.
 *
 * @retval:  				None
 *
 * @note:    				Random is still deterministic.
 */
static void initLiquid(void)
{
	srand(getSysTick());
	for(uint8_t i = 0; i < LIQUID_PARTICLES_NUMBER; i++)
	{
		liquidParticles[i].vx = 0;
		liquidParticles[i].vy = 0;

		do
		{
			liquidParticles[i].x = rand() % 8;
			liquidParticles[i].y = rand() % 8;
		}
		while(isParticleThere(liquidParticles[i].x, liquidParticles[i].y, i));
	}

	return;
}


/*
 * @brief:					Initializes the game: peripherals, timer and the liquid simulation engine.
 *
 * @retval:  				None
 *
 * @note:
 */
void initParticleGame(void)
{
	initSysTick();
	initMPU6050();
	initMAX7219();
	initTim2();
	initLiquid();

	return;
}

/*
 * @brief:					Renders out the current matrix to MAX7219.
 *
 * @retval:  				None
 *
 * @note:					Clearin, setting & rendering
 */
static void renderLiquid(void)
{
	clearMatrix();

	for(int i = 0; i < LIQUID_PARTICLES_NUMBER; i++)
	{
		setPixel((uint8_t)liquidParticles[i].x, (uint8_t)liquidParticles[i].y, 1);
	}

	renderMAX7219();

	return;
}

/*
 * @brief:					Checks the matrix boundaries.
 *
 * @param particle:			Pointer to particle.
 *
 * @retval:  				None
 *
 * @note:
 */
static void boundaryCheck(liquidParticle *particle)
{
	if(particle->x < 0)
	{
		particle->x = 0;
		particle->vx *= -0.5;
	}
	else if(particle->x > DAISY_MATRIX_SIZE_X-1)
	{
		particle->x = DAISY_MATRIX_SIZE_X-1;
		particle->vx *= -0.5;
	}

	if(particle->y < 0)
	{
		particle->y = 0;
		particle->vy *= -0.5;
	}
	else if(particle->y > DAISY_MATRIX_SIZE_Y-1)
	{
		particle->y = DAISY_MATRIX_SIZE_Y-1;
		particle->vy *= -0.5;
	}

	return;
}

/*
 * @brief:					Calculates the euclidean distance.
 *
 * @param particleA:		Pointer to particle A.
 *
 * @param particleB:		Pointer to particle B.
 *
 * @param distanceVector:	Distance vector of particle A and particle B;
 *
 * @retval:  				None
 *
 * @note:    				- sqrtf() function was used before, but it is  an expensive calculation
 * 							- That is the reason of using 'MINIMUM_DISTANCE * MINIMUM_DISTANCE' in processLiquid().
 */
static float calculateDistanceVector(liquidParticle *particleA, liquidParticle *particleB, Vector *distanceVector)
{
	// Calculate X, Y coordinate of the distance vector
	distanceVector->x = particleA->x - particleB->x;
	distanceVector->y = particleA->y - particleB->y;
	// Euclidean distance: d = sqrt(dx^2 + dy^2)
	//return sqrtf(distanceVector->x * distanceVector->x + distanceVector->y * distanceVector->y); --> too expensive for MCU withhout FPU
	float dist = distanceVector->x * distanceVector->x + distanceVector->y * distanceVector->y;

	return dist;
}

/*
 * @brief:				Calculates the unit vector.
 *
 * @param inputVector:	Pointer to the vector to be normalized.
 *
 * @param unitVector:	Pointer to the vector that will store the resulting unit vector.
 *
 * @retval:  			None
 *
 */
static void normalize(Vector *inputVector, Vector *unitVector)
{
	float magnitude = sqrtf(inputVector->x * inputVector->x + inputVector->y * inputVector->y);
	if(magnitude > 0.0001f)	//prevents division by 0
	{
		unitVector->x = inputVector->x/magnitude;
		unitVector->y = inputVector->y/magnitude;
	}
	else
	{
		unitVector->x = 0.0f;
		unitVector->y = 0.0f;
	}

	return;
}

/*
 * @brief:			Updates velocity and position of a particle based on acceleration.
 *
 * @param particle:	Pointer to the particle to update.
 *
 * @retval:  		None
 *
 * @note:    		Velocity is integrated using simple Euler integration.
 */
static void updateVelPos(liquidParticle *particle)
{
	// Update velocity
	particle->vx += DT * (gravity.x);
	particle->vy += DT * (gravity.y);
	// Add damping
	particle->vx *= GRAVITY_DAMPING;
	particle->vy *= GRAVITY_DAMPING;
	// Update position
	particle->x += DT * (particle->vx);
	particle->y += DT * (particle->vy);

	return;
}

/*
 * @brief:			Updates the gravity vector based on data from sensor.
 *
 * @param mpu:		Pointer to the sensor's data.
 *
 * @param g:		Pointer to the gravity to update.
 *
 * @retval:  		None
 *
 * @note:
 */
static void updateGravity(dataMPU6050 *mpu, Vector *g)
{
	// (-1.0f) needed, because of the sensor's orientation
	g->y = (-1.0f) * DEG2RAD(mpu->roll);
	g->x = DEG2RAD(mpu->pitch);

	if(g->y < 0.1f && g->y > -0.1f) g->y = 0.0f;
	if(g->x < 0.1f && g->x > -0.1f) g->x = 0.0f;

	return;
}

/*
 * @brief:				Resolves an elastic collision between two particles.
 *
 * @param particleA:	Pointer to particle A.
 *
 * @param particleB:	Pointer to particle B.
 *
 * @param distance:		Distance between particle A and particle B;
 *
 * @retval:  			None
 *
 * @note:				Separates overlapping particles, exchange momentum, assumes 2D particles.
 */
static void processCollsion(liquidParticle *particleA, liquidParticle *particleB, float distance)
{
	Vector unitVector;
	// Calculate the unit vector of distanceVector -> unit vector gives ONLY direction /wo magnitude
	normalize(&distanceVector, &unitVector);
	// newDist = dist + moveA + moveB	^ moveA = overlap ^ moveB = overlap ----> newDist = dist + 2 * overlap -----> overlap = 0.5 * (newD - d)
	float overlap = 0.5f * (MINIMUM_DISTANCE - distance);
	// Moving the 2 colliding particle to different direction
	particleA->x -= overlap * unitVector.x;
	particleA->y -= overlap * unitVector.y;
	particleB->x += overlap * unitVector.x;
	particleB->y += overlap * unitVector.y;
	// Calculating the velocity component along collison: velocitiy of particle A in the current direction
	float vA = particleA->vx * unitVector.x + particleA->vy * unitVector.y;
	float vB = particleB->vx * unitVector.x + particleB->vy * unitVector.y;
	// Avarage velocity -> for momentum exchange
	float avg = (vA + vB) / 2.0f;
	// Ellastic collision
	particleA->vx += (avg - vA) * unitVector.x;
	particleA->vy += (avg - vA) * unitVector.y;
	particleB->vx += (avg - vB) * unitVector.x;
	particleB->vy += (avg - vB) * unitVector.y;

	return;
}

/*
 * @brief:				Generates surface tension by the attraction of particles.
 *
 * @param particleA:	Pointer to particle A.
 *
 * @param particleB:	Pointer to particle B.
 *
 * @param distance:		Distance between particle A and particle B;
 *
 * @retval:  			None
 *
 * @note:
 */
static void processSurfaceTension(liquidParticle *particleA, liquidParticle *particleB, float distance)
{
	Vector unitVector;
	// Calculate the unit vector of distanceVector -> unit vector gives ONLY direction /wo magnitude
	normalize(&distanceVector, &unitVector);

	float force = ATTRACTION_STRENGTH * (ATTRACTION_RADIUS - distance);

	particleA->vx += force * unitVector.x;
	particleA->vy += force * unitVector.y;
	particleB->vx -= force * unitVector.x;
	particleB->vy -= force * unitVector.y;

	return;
}

/*
 * @brief:				Main simulation engine for particle-based liquid system.
 *
 * @retval:  			None
 *
 * @note:				Reads the latest data (MPU6050), updates all particle velocites and positions,
 * 						resolves boundary collisions, renders the updated LED matrix.
 *
 * @note:				Designed to be called periodically in a timer-based loop.
 */
void processLiquid(void)
{
	if(timer2_flag) timer2_flag = 0;
	else return;

	getDataMPU6050(&mpu_liquid);
	updateGravity(&mpu_liquid, &gravity);
	// Update every particles with the recently read data from MPU6050
	for(int i = 0; i < LIQUID_PARTICLES_NUMBER; i++)
	{
		updateVelPos(&liquidParticles[i]);
		boundaryCheck(&liquidParticles[i]);
	}
	// Iterate through every particle pairs (particle - particle collision)
	for(int i = 0; i < LIQUID_PARTICLES_NUMBER; i++)
	{
		for(int j = i+1; j < LIQUID_PARTICLES_NUMBER; j++)
		{
			float distance = calculateDistanceVector(&liquidParticles[j], &liquidParticles[i], &distanceVector);
			if(distance < 0.001f) continue;
			//If particles are closer than allowed min distance then push them appart
			if(distance < MINIMUM_DISTANCE * MINIMUM_DISTANCE && distance > 0.01f)
			{
				processCollsion(&liquidParticles[i], &liquidParticles[j], distance);
			}
			else if(distance < ATTRACTION_RADIUS)
			{
				processSurfaceTension(&liquidParticles[i], &liquidParticles[j], distance);
			}
		}
	}
	// Render updated particle positions to LED matrix
	renderLiquid();

	return;
}


