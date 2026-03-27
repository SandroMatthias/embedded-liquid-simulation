#ifndef INC_REGMPU6050_H_
#define INC_REGMPU6050C_H_

#define ADDR_DEVICE  				(0x68 << 1)

#define REG_ACCEL_XOUT_H			0x3B

#define REG_PWR_MGMT_1				0x6B		// Wake up device
#define REG_SMPLRT_DIV				0x19		// Sample rate divider
#define REG_CONFIG					0x1A
#define REG_GYRO_CONFIG				0x1B
#define REG_ACCEL_CONFIG			0x1C

#define DATA_WAKE_UP				0x00
#define DATA_SMPLRATE_DIV_7 		0x07		// Samplerate = gyroscope outputrate / (1 + DATA_SMPLRATE_DIV_X) = 8kHz / (1 +7)
#define DATA_CONFIG_NO_SYNC_NO_LPF	0x00		// no external sync, no low pass filter
#define DATA_GYRO_CONFIG_250		0x00		// +-250 °/s (deg/sec)
#define DATA_ACCEL_CONFIG_2G		0x00		// +-2g

#define	MPU_DATA_SIZE				14			// 2 * 3 + 2 * 3 +2 -> gyro L & H bytes (x, y, z) + accel L & H bytes (x, y, z) + temp L H

#endif
