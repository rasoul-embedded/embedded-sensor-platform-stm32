/*
 * lsm6dso32.h
 *
 *  Created on: Apr 1, 2026
 *      Author: Rasoul
 */

#ifndef INC_LSM6DSO32_H_
#define INC_LSM6DSO32_H_

#include "stm32f407xx_i2c_driver.h"
#include <stdint.h>

/* 7-bit I2C address */
#define LSM6DSO32_I2C_ADDR                 0x6A

/* Register addresses */
#define LSM6DSO32_REG_WHO_AM_I            0x0F
#define LSM6DSO32_REG_CTRL1_XL            0x10
#define LSM6DSO32_REG_CTRL2_G             0x11
#define LSM6DSO32_REG_CTRL3_C             0x12

#define LSM6DSO32_REG_OUTX_L_G            0x22
#define LSM6DSO32_REG_OUTX_H_G            0x23
#define LSM6DSO32_REG_OUTY_L_G            0x24
#define LSM6DSO32_REG_OUTY_H_G            0x25
#define LSM6DSO32_REG_OUTZ_L_G            0x26
#define LSM6DSO32_REG_OUTZ_H_G            0x27

#define LSM6DSO32_REG_OUTX_L_A     		  0x28
#define LSM6DSO32_REG_OUTX_H_A		      0x29
#define LSM6DSO32_REG_OUTY_L_A		      0x2A
#define LSM6DSO32_REG_OUTY_H_A		      0x2B
#define LSM6DSO32_REG_OUTZ_L_A		      0x2C
#define LSM6DSO32_REG_OUTZ_H_A		      0x2D

/* Expected device ID */
#define LSM6DSO32_WHO_AM_I_VALUE          0x6C

/* Basic configuration values */
#define LSM6DSO32_CTRL3_C_IF_INC          0x04
#define LSM6DSO32_CTRL1_XL_104HZ_4G       0x4A
#define LSM6DSO32_CTRL2_G_104HZ_250DPS    0x40

#define LSM6DSO32_ACCEL_SENS_4G   		   0.000122f
#define LSM6DSO32_GYRO_SENS_250DPS         0.00875f
typedef struct
{
    I2C_Handle_t *hi2c;
    uint8_t address;
} LSM6DSO32_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} LSM6DSO32_AxesRaw_t;

typedef struct
{
    float x;
    float y;
    float z;
} LSM6DSO32_AxesG_t;

typedef struct
{
    float x;
    float y;
    float z;
} LSM6DSO32_AxesDps_t;

I2C_Status_t LSM6DSO32_ReadReg(LSM6DSO32_t *dev, uint8_t reg, uint8_t *data);
I2C_Status_t LSM6DSO32_WriteReg(LSM6DSO32_t *dev, uint8_t reg, uint8_t data);
uint8_t LSM6DSO32_Init(LSM6DSO32_t *dev);
uint8_t LSM6DSO32_CheckID(LSM6DSO32_t *dev);
I2C_Status_t LSM6DSO32_ReadRegs(LSM6DSO32_t *dev, uint8_t startReg, uint8_t *data, uint8_t len);
uint8_t LSM6DSO32_ReadAccelRaw(LSM6DSO32_t *dev, LSM6DSO32_AxesRaw_t *accel);
uint8_t LSM6DSO32_ReadAccelG(LSM6DSO32_t *dev, volatile LSM6DSO32_AxesG_t *accel_g);
uint8_t LSM6DSO32_ReadGyroRaw(LSM6DSO32_t *dev, LSM6DSO32_AxesRaw_t *gyro);
uint8_t LSM6DSO32_ReadGyroDPS(LSM6DSO32_t *dev, volatile LSM6DSO32_AxesDps_t *gyro_dps);

#endif /* INC_LSM6DSO32_H_ */
