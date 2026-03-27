#include "kalman_filter.h"

void initKalman(Kalman *kalman)
{
    kalman->angle = 0;
    kalman->bias = 0;				// bias - drift

    kalman->P[0][0] = 0;
    kalman->P[0][1] = 0;
    kalman->P[1][0] = 0;
    kalman->P[1][1] = 0;

    //Slow respond -> increase noiseAngle
    kalman->noiseAngle = 0.02;
    //drifts -> increase noiseBias
    kalman->noiseBias  = 0.003;
    //jitters->→ increase noiseMeasurement
    kalman->noiseMeasurement = 0.03;
}

float KalmanUpdate(Kalman *kalman, float accelerometerAngle, float gyroscopeRate, float dt)
{
							/* 1. PREDICTION - data used from gyroscope to calculate the angle */
	// x - state vector
	kalman->rate = gyroscopeRate - kalman->bias;																// Predicted rate value = measured value (from gyro) - calculated noise
	kalman->angle += dt * kalman->rate;																			// Integrating

							/* 2. UPDATE */
	//	Covariance matrix predicton: After prediction the angle and bias variance (uncertainty)
	kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->noiseAngle);	// Variance - updates angle uncertainty
	kalman->P[0][1] -= dt * kalman->P[1][1];																	// Covariance - correlation between angle and bias
	kalman->P[1][0] -= dt * kalman->P[1][1];																	// Covariance - correlation between angle and bias
	kalman->P[1][1] += kalman->noiseBias * dt;																	// Variance - updates bias uncertainty
	//	Kalman gain calculations K = P / (P + R)
    float totalMeasUncertainty = kalman->P[0][0] + kalman->noiseMeasurement;									// How uncertain we are in the measurement (angle variance + noise)

    float angleKalmanGain = kalman->P[0][0] / totalMeasUncertainty;												/* Kalman Gain - determines how much the system updates its internal state
    																											   estimate based on a new measurement versus its previous prediction */
    float biasKalmanGain = kalman->P[1][0] / totalMeasUncertainty;												// How much the system corrects the gyro drift & the angle
    // Measurement update
    float measError = accelerometerAngle - kalman->angle;														// Data used from accelometer to calculate the angle
    kalman->angle += angleKalmanGain * measError;
    kalman->bias  += biasKalmanGain * measError;
    // Update the covariance matrix
    kalman->P[0][0] -= angleKalmanGain * kalman->P[0][0];
    kalman->P[0][1] -= angleKalmanGain * kalman->P[0][1];
    kalman->P[1][0] -= biasKalmanGain * kalman->P[0][0];
    kalman->P[1][1] -= biasKalmanGain * kalman->P[0][1];

	return  kalman->angle;
}
