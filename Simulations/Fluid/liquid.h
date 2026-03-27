#ifndef INC_LIQUID_H_
#define INC_LIQUID_H_

#define LIQUID_PARTICLES_NUMBER	30		//The number of particles created in the simulation.
#define GRAVITY_DAMPING			0.95f
#define MINIMUM_DISTANCE		1.1f	//Minimum distance between two particles.
#define ATTRACTION_RADIUS		1.8f
#define ATTRACTION_STRENGTH		0.01f

typedef struct
{
	float x;
	float y;
}Vector;

void initParticleGame(void);
void processLiquid(void);

#endif
