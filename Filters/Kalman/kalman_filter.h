#ifndef INC_KALMAN_FILTER_H_
#define INC_KALMAN_FILTER_H_

typedef struct
{
	// State
	float angle;			// Estimated roll/pitch (fused angle after Kalman correction)
	float bias;				// Gyro bias (offset) -> Gyro output = true_rate + bias + noise
							// Estimates the bias automatically and subtracts it
	float rate; 			// Bias-corrected angular velocity -> real angular velocity used for prediction

	// Uncertainty
	float P[2][2];			/* Covariance matrix: P00 - uncertainty  of angle
												  P11 - uncertainty of bias
												  P01 - correlation between angle and bias
												  P10 - correlation between bias and angle */
	// Noise Parameters
	float noiseAngle;
	float noiseBias;
	float noiseMeasurement;
}Kalman;

void initKalman(Kalman *kalman);
float KalmanUpdate(Kalman *kalman, float accelerometerAngle, float gyroscopeRate, float dt);

#endif
