#include "MPU6050.h"
#include "regMPU6050.h"
#include "../../BSP/I2C/I2C.h"
#include "../../Filters/Kalman/kalman_filter.h"
//#include "../../Core/Inc/tim2.h"
#include "../../Utilities/timeout.h"

#include <math.h>

Kalman KalmanY, KalmanX;

void initMPU6050(void)
{
	initI2C();

	writeRegI2C(ADDR_DEVICE, REG_PWR_MGMT_1, DATA_WAKE_UP);
	timeOut(100);

	writeRegI2C(ADDR_DEVICE, REG_SMPLRT_DIV, DATA_SMPLRATE_DIV_7);
	timeOut(100);

	writeRegI2C(ADDR_DEVICE, REG_CONFIG, DATA_CONFIG_NO_SYNC_NO_LPF);
	timeOut(100);

	writeRegI2C(ADDR_DEVICE, REG_GYRO_CONFIG, DATA_GYRO_CONFIG_250);	// ±250 deg/s -> 131 lsb sensitivity
	timeOut(100);

	writeRegI2C(ADDR_DEVICE, REG_ACCEL_CONFIG, DATA_ACCEL_CONFIG_2G);	// 2g -> 16384 lsb sensitivity
	timeOut(100);

	initKalman(&KalmanY);
	initKalman(&KalmanX);

	__enable_irq();

	return;
}

void readAllMPU6050(dataMPU6050 *mpu)
{
    uint8_t buffer[MPU_DATA_SIZE];

    readRegI2C(ADDR_DEVICE, REG_ACCEL_XOUT_H, buffer, MPU_DATA_SIZE);

    mpu->rawAccelX = (buffer[0] << 8 | buffer[1]);
    mpu->rawAccelY = (buffer[2] << 8 | buffer[3]);
    mpu->rawAccelZ = (buffer[4] << 8 | buffer[5]);

    mpu->rawTemp   = (buffer[6] << 8 | buffer[7]);

    mpu->rawGyroX  = (buffer[8] << 8 | buffer[9]);
    mpu->rawGyroY  = (buffer[10] << 8 | buffer[11]);
    mpu->rawGyroZ  = (buffer[12] << 8 | buffer[13]);

    return;
}

void convertAllMPU6050(dataMPU6050 *mpu)
{
	mpu->convAccelX = mpu->rawAccelX / 16384.0f;
	mpu->convAccelY = mpu->rawAccelY / 16384.0f;
	mpu->convAccelZ = mpu->rawAccelZ / 16384.0f;

	mpu->convGyroX = mpu->rawGyroX / 131.0f;
	mpu->convGyroY = mpu->rawGyroY / 131.0f;
	mpu->convGyroZ = mpu->rawGyroZ / 131.0f;

	return;
}


void calculateRoll(dataMPU6050 *mpu)
{
	//roll relative to gravity vector magnitude
	float norm = sqrtf(mpu->convAccelX*mpu->convAccelX +
	                   mpu->convAccelY*mpu->convAccelY +
	                   mpu->convAccelZ*mpu->convAccelZ);
	//Board upside down
	float tmpRoll = RAD2DEG(asinf(mpu->convAccelY / norm));
	mpu->roll = KalmanUpdate(&KalmanY, tmpRoll, mpu->convGyroY, DT);
	return;
}

void calculatePitch(dataMPU6050 *mpu)
{
	// Pitch correction needed, because of the board (upside down)
	float tmpPitch = PITCH_CORRECTION + RAD2DEG(atan2f(-mpu->convAccelX, sqrtf((mpu->convAccelY * mpu->convAccelY) + (mpu->convAccelZ * mpu->convAccelZ))));
	mpu->pitch = KalmanUpdate(&KalmanX, tmpPitch, mpu->convGyroX,  DT);
	return;
}

void getDataMPU6050(dataMPU6050 *mpu)
{
	readAllMPU6050(mpu);
	convertAllMPU6050(mpu);
	calculateRoll(mpu);
	calculatePitch(mpu);

	return;
}

