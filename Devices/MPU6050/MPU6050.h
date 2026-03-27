#ifndef INC_MPU6050_H_
#define INC_MPU6050C_H_

#include <stdint.h>

#define RAD2DEG(x) ((x) * 180.0f / M_PI)
#define DEG2RAD(y) ((y) / (180.0f / M_PI))

// Threshold values -> MPU6050 is in breadboard & head downwards
#define PITCH_CORRECTION	60		// MPU is on my breadboard in 45 degree
#define THRESHOLD			5

typedef struct
{
	/* RAW DATA */
	int16_t rawAccelX, rawAccelY, rawAccelZ;
	int16_t rawGyroX, rawGyroY, rawGyroZ;
	int16_t rawTemp;

	/* CONVERTED DATA */
	float convAccelX, convAccelY, convAccelZ;		// values in g
	float convGyroX, convGyroY, convGyroZ;			// in °/s
	float convTemp;									// in °C

	/* CALCULATED DATA */
	float roll;
	float pitch;

	/* FILTERED DATA */
	float filteredRoll;
	float filteredPitch;

} dataMPU6050;

typedef enum {UP, DOWN, RIGHT, LEFT, NONE} Movement;

void initMPU6050(void);

void readAllMPU6050(dataMPU6050 *mpu);
void convertAllMPU6050(dataMPU6050 *mpu);
void calculateRoll(dataMPU6050 *mpu);
void calculatePitch(dataMPU6050 *mpu);
void getDataMPU6050(dataMPU6050 *mpu);

#endif
