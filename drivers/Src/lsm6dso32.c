/*
 * lsm6dso32.c
 *
 *  Created on: Apr 1, 2026
 *      Author: Rasoul
 */


#include "lsm6dso32.h"

I2C_Status_t LSM6DSO32_ReadReg(LSM6DSO32_t *dev, uint8_t reg, uint8_t *data)
{
    I2C_Status_t status;

    // Send register address using repeated start enabled
    status = I2C_MasterSendData(dev->hi2c, &reg, 1, dev->address, I2C_ENABLE_SR);
    if (status != I2C_OK)
    {
        return status;
    }

    // Read one byte and finish with STOP
    status = I2C_MasterReceiveData(dev->hi2c, data, 1, dev->address, I2C_DISABLE_SR);

    return status;
}

I2C_Status_t LSM6DSO32_WriteReg(LSM6DSO32_t *dev, uint8_t reg, uint8_t data)
{
    uint8_t tx[2];

    // First byte should be register address
    tx[0] = reg;

    // Second byte should be data to write
    tx[1] = data;

    // Write both bytes in one transaction, no repeated start needed
    return I2C_MasterSendData(dev->hi2c, tx, 2, dev->address, I2C_DISABLE_SR);
}

uint8_t LSM6DSO32_CheckID(LSM6DSO32_t *dev)
{
    uint8_t id = 0;
    I2C_Status_t status;

    status = LSM6DSO32_ReadReg(dev,LSM6DSO32_REG_WHO_AM_I, &id);
    if (status != I2C_OK)
    {
        return 0;
    }

    if (id == LSM6DSO32_WHO_AM_I_VALUE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t LSM6DSO32_Init(LSM6DSO32_t *dev)
{
    I2C_Status_t status;

    // Make sure the sensor is really present
    if (LSM6DSO32_CheckID(dev) != 1U)
    {
        return 0U;
    }

    // Enable IF_INC so multi-byte register addresses auto increment.
    status = LSM6DSO32_WriteReg(dev,LSM6DSO32_REG_CTRL3_C, LSM6DSO32_CTRL3_C_IF_INC );
    if (status != I2C_OK)
    {
        return 0U;
    }

    // Enable accelerometer at 104 Hz, +/-4g
    status = LSM6DSO32_WriteReg(dev, LSM6DSO32_REG_CTRL1_XL, LSM6DSO32_CTRL1_XL_104HZ_4G);
    if (status != I2C_OK)
    {
        return 0U;
    }

    // Enable gyroscope at 104 Hz, 250 dps
    status = LSM6DSO32_WriteReg(dev, LSM6DSO32_REG_CTRL2_G, LSM6DSO32_CTRL2_G_104HZ_250DPS);
    if (status != I2C_OK)
    {
        return 0U;
    }

    return 1U;
}


I2C_Status_t LSM6DSO32_ReadRegs(LSM6DSO32_t *dev, uint8_t startReg, uint8_t *data, uint8_t len)
{
    I2C_Status_t status;

    status = I2C_MasterSendData(dev->hi2c, &startReg, 1, dev->address, I2C_ENABLE_SR);
    if (status != I2C_OK)
    {
        return status;
    }

    status = I2C_MasterReceiveData(dev->hi2c, data, len, dev->address, I2C_DISABLE_SR);
    return status;
}


uint8_t LSM6DSO32_ReadAccelRaw(LSM6DSO32_t *dev, LSM6DSO32_AxesRaw_t *accel)
{
    uint8_t raw[6];
    I2C_Status_t status;

    status = LSM6DSO32_ReadRegs(dev, LSM6DSO32_REG_OUTX_L_A, raw, 6);
    if (status != I2C_OK)
    {
        return 0U;
    }

    accel->x = (int16_t)((raw[1] << 8) | raw[0]);
    accel->y = (int16_t)((raw[3] << 8) | raw[2]);
    accel->z = (int16_t)((raw[5] << 8) | raw[4]);

    return 1U;
}


uint8_t LSM6DSO32_ReadAccelG(LSM6DSO32_t *dev, volatile LSM6DSO32_AxesG_t *accel_g)
{
    LSM6DSO32_AxesRaw_t raw;
    uint8_t status;

    status = LSM6DSO32_ReadAccelRaw(dev, &raw);
    if (status != 1U)
    {
        return 0U;
    }


    // Replace ACCEL_SENS_4G with the correct sensitivity value for your selected +/-4g setting

    accel_g->x = raw.x * LSM6DSO32_ACCEL_SENS_4G;
    accel_g->y = raw.y * LSM6DSO32_ACCEL_SENS_4G;
    accel_g->z = raw.z * LSM6DSO32_ACCEL_SENS_4G;

    return 1U;
}

uint8_t LSM6DSO32_ReadGyroRaw(LSM6DSO32_t *dev, LSM6DSO32_AxesRaw_t *gyro)
{
    uint8_t raw[6];
    I2C_Status_t status;

    status = LSM6DSO32_ReadRegs(dev, LSM6DSO32_REG_OUTX_L_G, raw, 6);
    if (status != I2C_OK)
    {
        return 0U;
    }

    gyro->x = (int16_t)((raw[1] << 8) | raw[0]);
    gyro->y = (int16_t)((raw[3] << 8) | raw[2]);
    gyro->z = (int16_t)((raw[5] << 8) | raw[4]);

    return 1U;
}

uint8_t LSM6DSO32_ReadGyroDPS(LSM6DSO32_t *dev, volatile LSM6DSO32_AxesDps_t *gyro_dps)
{
    LSM6DSO32_AxesRaw_t raw;
    uint8_t status;

    status = LSM6DSO32_ReadGyroRaw(dev, &raw);
    if (status != 1U)
    {
        return 0U;
    }


    gyro_dps->x = raw.x * LSM6DSO32_GYRO_SENS_250DPS;
    gyro_dps->y = raw.y * LSM6DSO32_GYRO_SENS_250DPS;
    gyro_dps->z = raw.z * LSM6DSO32_GYRO_SENS_250DPS;

    return 1U;
}
