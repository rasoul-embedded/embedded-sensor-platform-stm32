#include "stm32f407xx_gpio_driver.h"
#include "stm32f407xx_i2c_driver.h"
#include "stm32f407xx_usart_driver.h"
#include "stm32f407xx_tim_driver.h"
#include "stm32f407xx_dma_driver.h"
#include "lsm6dso32.h"
#include <stdio.h>
#include <string.h>
#include <math.h>




static I2C_Handle_t i2c1Handle;
static USART_Handle_t usart2Handle;
static TIM_Handle_t tim2Handle;
static DMA_Handle_t usart2TxDmaHandle;

LSM6DSO32_AxesG_t accel_g;
LSM6DSO32_AxesDps_t gyro_dps;

volatile uint8_t sample_flag = 0;
static uint32_t sample_count= 0;
static uint8_t filter_initialized = 0;
static float roll = 0, pitch = 0;
static uint8_t USART2_DMA_Send(uint8_t *pTxBuffer, uint32_t Len);


LSM6DSO32_A_t a;
LSM6DSO32_G_t g;

LSM6DSO32_AF_t a_f;
LSM6DSO32_GF_t g_f;

#define ALPHA 		0.1f
#define DT 0.01f
#define RAD_TO_DEG 57.2958f
#define USART2_DMA_TX_IDLE  0U
#define USART2_DMA_TX_BUSY  1U
char msg[128];
static volatile uint8_t usart2_dma_tx_busy = USART2_DMA_TX_IDLE;



static void USART2_TX_DMA_Init(void)
{
    /*
     * USART2_TX mapping:
     * DMA controller = DMA1
     * Stream         = 6
     * Channel        = 4
     */

    usart2TxDmaHandle.pDMAx = DMA1;
    usart2TxDmaHandle.pStream = DMA1_Stream6;
    usart2TxDmaHandle.StreamNumber = 6U;

    /*
     * Configure DMA stream
     */
    usart2TxDmaHandle.DMA_Config.Channel = DMA_CHANNEL_4;
    usart2TxDmaHandle.DMA_Config.Direction = DMA_DIR_M2P;

    usart2TxDmaHandle.DMA_Config.PeriphInc = DMA_PINC_DISABLE;
    usart2TxDmaHandle.DMA_Config.MemInc = DMA_MINC_ENABLE;

    usart2TxDmaHandle.DMA_Config.PeriphDataSize = DMA_PSIZE_BYTE;
    usart2TxDmaHandle.DMA_Config.MemDataSize = DMA_MSIZE_BYTE;

    usart2TxDmaHandle.DMA_Config.Mode = DMA_MODE_NORMAL;
    usart2TxDmaHandle.DMA_Config.Priority = DMA_PL_LOW;

    /*
     * Initialize DMA stream
     */
    DMA_Init(&usart2TxDmaHandle);

    /*
     * Clear old flags before first use
     */
    DMA_ClearFlags(&usart2TxDmaHandle);

    /*
     * Configure DMA IRQ in NVIC
     */
    DMA_IRQPriorityConfig(IRQ_NO_DMA1_STREAM6, 15);
    DMA_IRQInterruptConfig(IRQ_NO_DMA1_STREAM6, ENABLE);
}

static uint8_t USART2_DMA_Send(uint8_t *pTxBuffer, uint32_t Len)
{
    /*
     * 1. Reject invalid transfer
     */
    if (pTxBuffer == 0 || Len == 0)
    {
        return DMA_TX_ERROR;
    }

    /*
     * 2. If previous DMA TX is still running, do not overwrite/send
     */
    if (usart2_dma_tx_busy == USART2_DMA_TX_BUSY)
    {
        return DMA_TX_BUSY;
    }

    /*
     * 3. Mark USART2 DMA TX busy
     */
    usart2_dma_tx_busy = USART2_DMA_TX_BUSY;

    /*
     * 4. Make sure stream is disabled before configuring addresses/length
     */
    DMA_DisableStream(&usart2TxDmaHandle);

    /*
     * 5. Clear old DMA flags for Stream 6
     */
    DMA_ClearFlags(&usart2TxDmaHandle);

    /*
     * 6. Configure peripheral and memory addresses
     *
     * Peripheral address = address of USART2 data register
     * Memory address     = address of transmit buffer
     */
    DMA_ConfigAddresses(&usart2TxDmaHandle,
                        (uint32_t)&usart2Handle.pUSART->DR,
                        (uint32_t)pTxBuffer);

    /*
     * 7. Configure number of data items
     *
     * For UART TX:
     * MSIZE = byte
     * PSIZE = byte
     * therefore Len = number of bytes
     */
    DMA_SetDataLength(&usart2TxDmaHandle, Len);

    /*
     * 8. Enable USART DMA transmit request
     */
    USART2->CR3 |= USART_CR3_DMAT;

    /*
     * 9. Enable DMA stream
     */
    DMA_EnableStream(&usart2TxDmaHandle);

    return DMA_TX_OK;
}
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
    /*
     * If DMA is still sending the previous message,
     * do not overwrite msg[].
     */
    if (usart2_dma_tx_busy == DMA_TX_BUSY)
    {
        return;
    }

    int roll_i  = (int)(roll * 100);
    int pitch_i = (int)(pitch * 100);

    sprintf(msg,
            "%lu,%d,%d\r\n",
            sample_count++,
            roll_i,
            pitch_i);

    USART2_DMA_Send((uint8_t *)msg, strlen(msg));
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
    USART2_TX_DMA_Init();
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


    		imu_apply_calibration();
    		imu_apply_filter();
    		imu_compute_angles();
    		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_13, SET);
    		imu_process();
    		GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_13, RESET);
    	}
    }
}


void DMA1_Stream6_IRQHandler(void)
{
    /*
     * Check transfer complete flag for Stream 6
     */
    if (DMA_GetFlagStatus(&usart2TxDmaHandle, DMA_FLAG_TCIF2_6) == FLAG_SET)
    {
        /*
         * Clear all flags for Stream 6
         */
        DMA_ClearFlags(&usart2TxDmaHandle);
        /*
         * Disable stream after normal-mode transfer
         */
        DMA_DisableStream(&usart2TxDmaHandle);

        /*
         * Optional:
         * Disable USART DMA TX request
         */
        USART2->CR3 &= ~USART_CR3_DMAT;

        /*
         * Mark DMA TX free
         */
        usart2_dma_tx_busy = USART2_DMA_TX_IDLE;
        return;
    }

    /*
     * Optional error handling:
     * Check transfer error flag
     */
    if (DMA_GetFlagStatus(&usart2TxDmaHandle, DMA_FLAG_TEIF2_6) == FLAG_SET)
    {
        DMA_ClearFlags(&usart2TxDmaHandle);
        DMA_DisableStream(&usart2TxDmaHandle);
        USART2->CR3 &= ~USART_CR3_DMAT;
        usart2_dma_tx_busy = USART2_DMA_TX_IDLE;
    }
}
