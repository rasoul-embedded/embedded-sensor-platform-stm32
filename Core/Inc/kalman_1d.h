#ifndef INC_KALMAN_1D_H_
#define INC_KALMAN_1D_H_

/*
 * kalman_1d.h
 *
 * Purpose:
 * Reusable 1D Kalman filter for angle estimation.
 *
 * State:
 *   angle
 *   gyro bias
 *
 * Inputs:
 *   accelerometer angle
 *   gyro rate
 *   dt
 *
 * Output:
 *   estimated angle
 */

typedef struct
{
    /*
     * Estimated angle in degrees.
     */
    float angle;

    /*
     * Estimated gyro bias in degrees/second.
     */
    float bias;

    /*
     * Unbiased gyro rate:
     *
     * rate = new_rate - bias
     */
    float rate;

    /*
     * Error covariance matrix.
     *
     * P[0][0] -> angle uncertainty
     * P[1][1] -> bias uncertainty
     * P[0][1], P[1][0] -> relationship between angle and bias uncertainty
     */
    float P[2][2];

    /*
     * Process noise for angle.
     */
    float Q_angle;

    /*
     * Process noise for gyro bias.
     */
    float Q_bias;

    /*
     * Measurement noise.
     *
     * This represents how noisy the accelerometer angle is.
     */
    float R_measure;

} Kalman1D_t;


/*
 * Initialize Kalman filter.
 */
void Kalman1D_Init(Kalman1D_t *kf,
                   float initial_angle,
                   float q_angle,
                   float q_bias,
                   float r_measure);


/*
 * Update Kalman filter.
 *
 * new_angle:
 *   Accelerometer angle in degrees.
 *
 * new_rate:
 *   Gyroscope rate in degrees/second.
 *
 * dt:
 *   Sampling period in seconds.
 *
 * Return:
 *   Updated estimated angle in degrees.
 */
float Kalman1D_Update(Kalman1D_t *kf,
                      float new_angle,
                      float new_rate,
                      float dt);


/*
 * Getter functions.
 */
float Kalman1D_GetAngle(Kalman1D_t *kf);
float Kalman1D_GetBias(Kalman1D_t *kf);
float Kalman1D_GetRate(Kalman1D_t *kf);

#endif /* INC_KALMAN_1D_H_ */
