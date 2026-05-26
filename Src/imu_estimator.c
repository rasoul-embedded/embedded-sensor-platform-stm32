#include "imu_estimator.h"
#include <math.h>

#define COMP_ACCEL_WEIGHT 0.02f
#define RAD_TO_DEG 57.2958f

static float roll;
static float pitch;

static float roll_gyro;
static float pitch_gyro;

static float roll_acc;
static float pitch_acc;

void IMU_Estimator_Init(void)
{

    roll = 0.0f;
    pitch = 0.0f;

    roll_acc = 0.0f;
    pitch_acc = 0.0f;

    roll_gyro = 0.0f;
    pitch_gyro = 0.0f;
}

void IMU_Estimator_Update(float ax_mg,
                          float ay_mg,
                          float az_mg,
                          float gx_mdps,
                          float gy_mdps,
                          float dt)
{
    /*
     * 1. Compute accelerometer roll and pitch
     */
	roll_acc = atan2f(ay_mg, az_mg) * RAD_TO_DEG;

	pitch_acc = atan2f(-ax_mg, sqrtf(ay_mg * ay_mg + az_mg * az_mg)) * RAD_TO_DEG;

    /*
     * 2. Convert gyro from mdps to dps
     */
	float gx_dps = gx_mdps / 1000.0f;
	float gy_dps = gy_mdps / 1000.0f;

	/*
	 * 3. Calculate roll and pitch gyro
	 */
	roll_gyro  = roll + gx_dps * dt;
	pitch_gyro = pitch + gy_dps * dt;

    /*
     * 4. Complementary filter
     */
	roll  = (1.0f - COMP_ACCEL_WEIGHT) * roll_gyro + COMP_ACCEL_WEIGHT * roll_acc;
	pitch = (1.0f - COMP_ACCEL_WEIGHT) * pitch_gyro + COMP_ACCEL_WEIGHT * pitch_acc;
}

float IMU_Estimator_GetRoll(void)
{
    return roll;
}

float IMU_Estimator_GetPitch(void)
{
    return pitch;
}

float IMU_Estimator_GetRollAcc(void)
{
    return roll_acc;
}

float IMU_Estimator_GetPitchAcc(void)
{
    return pitch_acc;
}

float IMU_Estimator_GetRollGyro(void)
{
    return roll_gyro;
}

float IMU_Estimator_GetPitchGyro(void)
{
    return pitch_gyro;
}
