#include "stm32f407xx_gpio_driver.h"
#include "stm32f407xx_i2c_driver.h"
#include "stm32f407xx_usart_driver.h"
#include "stm32f407xx_tim_driver.h"
#include "lsm6dso32.h"
#include <stdio.h>
#include <string.h>
#include <math.h>




static I2C_Handle_t i2c1Handle;
static USART_Handle_t usart2Handle;
static TIM_Handle_t tim2Handle;

LSM6DSO32_AxesG_t accel_g;
LSM6DSO32_AxesDps_t gyro_dps;

volatile uint8_t sample_flag = 0;
static uint32_t sample_count= 0;
static uint8_t filter_initialized = 0;
static float roll = 0, pitch = 0;


LSM6DSO32_A_t a;
LSM6DSO32_G_t g;

LSM6DSO32_AF_t a_f;
LSM6DSO32_GF_t g_f;

#define ALPHA 		0.1f
#define DT 0.01f
#define RAD_TO_DEG 57.2958f

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

static void GPIODInit(void)
{
	GPIO_Handle_t gpiod;
    gpiod.pGPIOx = GPIOD;
    gpiod.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    gpiod.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
    gpiod.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    gpiod.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    gpiod.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    GPIO_Init(&gpiod);

    gpiod.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&gpiod);
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


static void imu_read_raw(LSM6DSO32_t *imu)
{
	LSM6DSO32_ReadAccelG(imu, &accel_g);
	LSM6DSO32_ReadGyroDPS(imu, &gyro_dps);
}

static void imu_apply_calibration(void)
{
	a.x = accel_g.x * 1000 - AX_BIAS;
	a.y = accel_g.y * 1000 - AY_BIAS;
	a.z = accel_g.z * 1000 - AZ_BIAS;

	g.x = gyro_dps.x * 1000 - GX_BIAS;
	g.y = gyro_dps.y * 1000 - GY_BIAS;
	g.z = gyro_dps.z * 1000 - GZ_BIAS;
}

static void imu_apply_filter(void)
{

	if (!filter_initialized)
	{
		a_f.x = a.x; a_f.y = a.y; a_f.z = a.z;
		g_f.x = g.x; g_f.y = g.y; g_f.z = g.z;
		filter_initialized = 1;
		return;
	}

	a_f.x = ALPHA * a.x + (1.0 - ALPHA)*a_f.x;
	a_f.y = ALPHA * a.y + (1.0 - ALPHA)*a_f.y;
	a_f.z = ALPHA * a.z + (1.0 - ALPHA)*a_f.z;


	g_f.x = ALPHA * g.x + (1.0 - ALPHA)*g_f.x;
	g_f.y = ALPHA * g.y + (1.0 - ALPHA)*g_f.y;
	g_f.z = ALPHA * g.z + (1.0 - ALPHA)*g_f.z;

}

static void imu_compute_angles(void)
{
    float roll_acc  = atan2f(a_f.y, a_f.z) * RAD_TO_DEG;
    float pitch_acc = atan2f(-a_f.x, sqrtf(a_f.y*a_f.y + a_f.z*a_f.z)) * RAD_TO_DEG;

    float gx_dps = g_f.x / 1000.0f;
    float gy_dps = g_f.y / 1000.0f;

    roll  = 0.98f * (roll + gx_dps * DT) + 0.02f * roll_acc;
    pitch = 0.98f * (pitch + gy_dps * DT) + 0.02f * pitch_acc;
}

static void imu_process(void)
{
	int roll_i  = (int)(roll * 100);
	int pitch_i = (int)(pitch * 100);

	sprintf(msg,
	       "%lu,%d,%d\r\n",
	       sample_count++,
	       roll_i,
	       pitch_i);

    USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));
}


int main(void)
{
    LSM6DSO32_t imu;

    I2C1_GPIOInit();
    I2C1_Init();

    USART2_GPIOInit();
    USART2_Init();

    TIM2_Init();

    GPIODInit();

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

    		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_12, SET);
    		imu_read_raw(&imu);
    		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_12, RESET);


    		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_13, SET);
    		imu_apply_calibration();
    		imu_apply_filter();
    		imu_compute_angles();
    		imu_process();
    		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_13, RESET);
    	}
    }
}
