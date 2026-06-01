#include "kalman_1d.h"

void Kalman1D_Init(Kalman1D_t *kf,
                   float initial_angle,
                   float q_angle,
                   float q_bias,
                   float r_measure)
{
    /*
     * Protect against null pointer.
     */
    if (kf == 0)
    {
        return;
    }

    /*
     * Initial angle.
     * Later, this can be initialized from accelerometer angle.
     */
    kf->angle = initial_angle;

    /*
     * Initial gyro bias estimate.
     * We start with zero because we do not know the bias yet.
     */
    kf->bias = 0.0f;

    /*
     * Initial unbiased rate.
     */
    kf->rate = 0.0f;

    /*
     * Initialize covariance matrix.
     *
     * Starting from zero means:
     *   we initially assume no uncertainty.
     *
     * Later, you can test non-zero initial covariance.
     */
    kf->P[0][0] = 0.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 0.0f;

    /*
     * Store tuning parameters.
     */
    kf->Q_angle = q_angle;
    kf->Q_bias = q_bias;
    kf->R_measure = r_measure;
}


float Kalman1D_Update(Kalman1D_t *kf,
                      float new_angle,
                      float new_rate,
                      float dt)
{
    /*
     * Protect against null pointer.
     */
    if (kf == 0)
    {
        return 0.0f;
    }

    /*
     * ============================================================
     * 1. Prediction step
     * ============================================================
     *
     * Remove estimated bias from gyro rate.
     */
    kf->rate = new_rate - kf->bias;

    /*
     * Predict angle using gyro integration.
     */
    kf->angle = kf->angle + kf->rate * dt;

    /*
     * Update covariance matrix.
     *
     * These equations describe how uncertainty grows during prediction.
     *
     * P = FPF^T + Q
     *
     * For the 1D angle+bias model, the expanded equations are:
     */
    kf->P[0][0] += dt * (dt * kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;

    /*
     * ============================================================
     * 2. Correction step
     * ============================================================
     *
     * Innovation / measurement residual:
     *
     * Difference between accelerometer angle and predicted angle.
     */
    float y = new_angle - kf->angle;

    /*
     * Innovation covariance.
     *
     * S tells us how uncertain the measurement comparison is.
     */
    float S = kf->P[0][0] + kf->R_measure;

    /*
     * Kalman gain.
     *
     * K[0] corrects angle.
     * K[1] corrects bias.
     */
    float K0 = kf->P[0][0] / S;
    float K1 = kf->P[1][0] / S;
    /*
     * Correct angle and bias using the measurement residual.
     */
    kf->angle = kf->angle + K0 * y;
    kf->bias  = kf->bias +  K1 * y;

    /*
     * Update covariance matrix after correction.
     *
     * Store old P values first because they are used in multiple equations.
     */
    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];

    kf->P[0][0] = kf->P[0][0] - K0 * P00_temp;
    kf->P[0][1] = kf->P[0][1] - K0 * P01_temp;
    kf->P[1][0] = kf->P[1][0] - K1 * P00_temp;
    kf->P[1][1] = kf->P[1][1] - K1 * P01_temp;

    /*
     * Return estimated angle.
     */
    return kf->angle;
}


float Kalman1D_GetAngle(Kalman1D_t *kf)
{
    if (kf == 0)
    {
        return 0.0f;
    }

    return kf->angle;
}


float Kalman1D_GetBias(Kalman1D_t *kf)
{
    if (kf == 0)
    {
        return 0.0f;
    }

    return kf->bias;
}


float Kalman1D_GetRate(Kalman1D_t *kf)
{
    if (kf == 0)
    {
        return 0.0f;
    }

    return kf->rate;
}
