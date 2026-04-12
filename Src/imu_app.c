#include "stm32f407xx_gpio_driver.h"
#include "stm32f407xx_i2c_driver.h"
#include "stm32f407xx_usart_driver.h"
#include "stm32f407xx_tim_driver.h"
#include "lsm6dso32.h"
#include <stdio.h>
#include <string.h>

static I2C_Handle_t i2c1Handle;
static USART_Handle_t usart2Handle;
static TIM_Handle_t tim2Handle;

volatile LSM6DSO32_AxesG_t accel_g;
volatile LSM6DSO32_AxesDps_t gyro_dps;
volatile uint8_t sample_flag = 0;

char msg[128];

static void I2C1_GPIOInit(void)
{
    GPIO_Handle_t gpio;
    gpio.pGPIOx = GPIOB;

    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_OD;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;   // I2C1_SCL
    GPIO_Init(&gpio);

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;   // I2C1_SDA
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

static void USART2_GPIOInit(void)
{
    GPIO_Handle_t gpio;
    gpio.pGPIOx = GPIOA;

    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 7;          // AF7 = USART2
    gpio.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;  // USART2_TX
    GPIO_Init(&gpio);

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;  // USART2_RX
    GPIO_Init(&gpio);
}

static void USART2_Init(void)
{
    usart2Handle.pUSART = USART2;
    usart2Handle.USART_Config.Mode = USART_MODE_TXRX;
    usart2Handle.USART_Config.Baud = USART_STD_BAUD_115200;
    usart2Handle.USART_Config.NoOfStopBits = USART_STOPBITS_1;
    usart2Handle.USART_Config.WordLength = USART_WORDLEN_8BITS;
    usart2Handle.USART_Config.ParityControl = USART_PARITY_DISABLE;
    usart2Handle.USART_Config.HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_Init(&usart2Handle);
}


static void TIM2_Init(void)
{
	tim2Handle.pTIMx = TIM2;
	tim2Handle.TIM_Config.AutoReload = 9;
	tim2Handle.TIM_Config.Prescaler = 15999;

	TIM_Init(&tim2Handle);
	TIM_IRQPriorityConfig(IRQ_NO_TIM2, 15);
	TIM_EnableUpdateInterrupt(TIM2, IRQ_NO_TIM2);
}

void TIM2_IRQHandler(void)
{
	if (tim2Handle.pTIMx->SR & TIM_SR_UIF)
	{
		tim2Handle.pTIMx->SR &= ~TIM_SR_UIF;
		sample_flag = 1;
	}
}


int main(void)
{
    LSM6DSO32_t imu;

    I2C1_GPIOInit();
    I2C1_Init();

    USART2_GPIOInit();
    USART2_Init();

    TIM2_Init();

    imu.address = LSM6DSO32_I2C_ADDR;
    imu.hi2c = &i2c1Handle;

    if (!LSM6DSO32_Init(&imu))
    {
        sprintf(msg, "LSM6DSO32 init failed\r\n");
        USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));

        while (1);
    }

    TIM_PeripheralControl(TIM2, ENABLE);
    sprintf(msg, "LSM6DSO32 init OK\r\n");
    USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));

    while (1)
    {
    	if (sample_flag)
    	{
    		sample_flag = 0;
			LSM6DSO32_ReadAccelG(&imu, &accel_g);
			LSM6DSO32_ReadGyroDPS(&imu, &gyro_dps);
	        sprintf(msg,
	                "AX=%d AY=%d AZ=%d | GX=%d GY=%d GZ=%d\r\n",
	                (int)(accel_g.x * 1000),
	                (int)(accel_g.y * 1000),
	                (int)(accel_g.z * 1000),
	                (int)(gyro_dps.x * 1000),
	                (int)(gyro_dps.y * 1000),
	                (int)(gyro_dps.z * 1000));

	        USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));
    	}
    }
}
