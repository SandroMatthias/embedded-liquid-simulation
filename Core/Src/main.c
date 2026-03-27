#include "../../Simulations/Fluid/liquid.h"

int main(void)
{
	initParticleGame();

	while(1)
	{
		processLiquid();
	}

	return 0;
}
