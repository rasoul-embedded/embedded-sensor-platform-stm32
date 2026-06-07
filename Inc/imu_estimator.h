#ifndef INC_IMU_ESTIMATOR_H_
#define INC_IMU_ESTIMATOR_H_

void IMU_Estimator_Init(void);

void IMU_Estimator_Update(float ax_mg,
                          float ay_mg,
                          float az_mg,
                          float gx_mdps,
                          float gy_mdps,
                          float dt);

float IMU_Estimator_GetRoll(void);
float IMU_Estimator_GetPitch(void);

float IMU_Estimator_GetRollAcc(void);
float IMU_Estimator_GetPitchAcc(void);
float IMU_Estimator_GetRollGyro(void);
float IMU_Estimator_GetPitchGyro(void);

float IMU_Estimator_GetRollKalman(void);
float IMU_Estimator_GetPitchKalman(void);

#endif
