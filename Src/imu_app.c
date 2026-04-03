#include "stm32f407xx_gpio_driver.h"
#include "lsm6dso32.h"

static I2C_Handle_t i2c1Handle;
volatile LSM6DSO32_AxesRaw_t accel;
volatile LSM6DSO32_AxesG_t accel_g;
volatile LSM6DSO32_AxesDps_t gyro_dps;


static void I2C1_GPIOInit(void)
{
    GPIO_Handle_t gpio;
    gpio.pGPIOx = GPIOB;

    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_OD;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&gpio);

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&gpio);
}

static void I2C1_Init(void)
{
    i2c1Handle.pI2Cx = I2C1;
    i2c1Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    i2c1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    i2c1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    i2c1Handle.I2C_Config.I2C_DeviceAddress = 0x61;

    I2C_Init(&i2c1Handle);
}



int main(void)
{
    uint8_t ok;
    LSM6DSO32_t imu;

    I2C1_GPIOInit();
    I2C1_Init();

    imu.address = LSM6DSO32_I2C_ADDR;
    imu.hi2c = &i2c1Handle;

    ok = LSM6DSO32_Init(&imu);

    while (1)
    {
        if (ok == 1U)
        {
            LSM6DSO32_ReadAccelG(&imu, &accel_g);
            LSM6DSO32_ReadGyroDPS(&imu, &gyro_dps);

        }
    }
}
