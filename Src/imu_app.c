#include "stm32f407xx_gpio_driver.h"
#include "stm32f407xx_i2c_driver.h"
#include "stm32f407xx_usart_driver.h"
#include "stm32f407xx_tim_driver.h"
#include "stm32f407xx_dma_driver.h"
#include "lsm6dso32.h"
#include "imu_estimator.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

/* ============================================================
 * Peripheral handles
 * ============================================================
 *
 * These handles store configuration and runtime state for each
 * low-level driver.
 */
static I2C_Handle_t i2c1Handle;
static USART_Handle_t usart2Handle;
static TIM_Handle_t tim2Handle;
static DMA_Handle_t usart2TxDmaHandle;

/* ============================================================
 * IMU raw data containers
 * ============================================================
 *
 * accel_g:
 *   Accelerometer values returned by sensor driver in g.
 *
 * gyro_dps:
 *   Gyroscope values returned by sensor driver in degrees/second.
 */
LSM6DSO32_AxesG_t accel_g;
LSM6DSO32_AxesDps_t gyro_dps;

/* ============================================================
 * Scheduler / timing variables
 * ============================================================
 */

/*
 * Set by TIM2 interrupt every 10 ms.
 *
 * The ISR only sets this flag. The main loop performs the actual
 * IMU reading, filtering, estimation, and logging.
 */
volatile uint8_t sample_flag = 0;

/*
 * Counts logged samples.
 *
 * Useful for detecting missing samples in CSV logs.
 */
static uint32_t sample_count = 0;

/*
 * Used to initialize the IIR low-pass filter output with the first sample.
 */
static uint8_t filter_initialized = 0;

/* ============================================================
 * IMU calibrated and filtered data
 * ============================================================
 *
 * a:
 *   Calibrated accelerometer values in mg.
 *
 * g:
 *   Calibrated gyroscope values in mdps.
 *
 * a_f:
 *   Filtered accelerometer values in mg.
 *
 * g_f:
 *   Filtered gyroscope values in mdps.
 */
LSM6DSO32_A_t a;
LSM6DSO32_G_t g;

LSM6DSO32_AF_t a_f;
LSM6DSO32_GF_t g_f;

/* ============================================================
 * Application constants
 * ============================================================
 */

/*
 * Low-pass filter coefficient.
 *
 * Smaller ALPHA:
 *   smoother signal, more delay
 *
 * Larger ALPHA:
 *   faster response, more noise
 */
#define ALPHA                   0.1f

/*
 * Sampling period.
 *
 * TIM2 is configured for 100 Hz:
 *   dt = 1 / 100 = 0.01 s
 */
#define DT                      0.01f

/*
 * UART DMA transmit state.
 */
#define USART2_DMA_TX_IDLE      0U
#define USART2_DMA_TX_BUSY      1U

/*
 * Single UART transmit buffer.
 *
 * DMA reads directly from this buffer, so it must not be overwritten
 * while a DMA transfer is active.
 */
char msg[128];

/*
 * Tracks whether UART DMA is currently transmitting msg[].
 */
static volatile uint8_t usart2_dma_tx_busy = USART2_DMA_TX_IDLE;

/*
 * Local prototype because this function is private to main.c.
 */
static uint8_t USART2_DMA_Send(uint8_t *pTxBuffer, uint32_t Len);

/* ============================================================
 * USART2 TX DMA initialization
 * ============================================================
 */

static void USART2_TX_DMA_Init(void)
{
    /*
     * USART2_TX DMA mapping for STM32F407:
     *
     *   USART2_TX -> DMA1 Stream 6 Channel 4
     *
     * Stream:
     *   The DMA hardware transfer engine.
     *
     * Channel:
     *   Selects which peripheral request is connected to the stream.
     */
    usart2TxDmaHandle.pDMAx = DMA1;
    usart2TxDmaHandle.pStream = DMA1_Stream6;
    usart2TxDmaHandle.StreamNumber = 6U;

    /*
     * Channel 4 connects DMA1 Stream 6 to USART2_TX requests.
     */
    usart2TxDmaHandle.DMA_Config.Channel = DMA_CHANNEL_4;

    /*
     * UART transmit sends data from memory buffer to USART2 data register.
     */
    usart2TxDmaHandle.DMA_Config.Direction = DMA_DIR_M2P;

    /*
     * USART2->DR address must remain fixed.
     * msg[] memory address must increment byte-by-byte.
     */
    usart2TxDmaHandle.DMA_Config.PeriphInc = DMA_PINC_DISABLE;
    usart2TxDmaHandle.DMA_Config.MemInc = DMA_MINC_ENABLE;

    /*
     * UART transmits one byte at a time.
     */
    usart2TxDmaHandle.DMA_Config.PeriphDataSize = DMA_PSIZE_BYTE;
    usart2TxDmaHandle.DMA_Config.MemDataSize = DMA_MSIZE_BYTE;

    /*
     * Normal mode:
     *   Send one message once, then stop.
     *
     * Circular mode would repeatedly transmit the same buffer, which is not
     * desired for CSV logging.
     */
    usart2TxDmaHandle.DMA_Config.Mode = DMA_MODE_NORMAL;

    /*
     * UART logging is lower priority than sensor acquisition/control.
     */
    usart2TxDmaHandle.DMA_Config.Priority = DMA_PL_LOW;

    /*
     * Configure the DMA stream.
     * This does not start a transfer yet.
     */
    DMA_Init(&usart2TxDmaHandle);

    /*
     * Clear any old flags before first use.
     */
    DMA_ClearFlags(&usart2TxDmaHandle);

    /*
     * Enable DMA1 Stream 6 interrupt.
     *
     * The IRQ handler releases msg[] after DMA transfer complete.
     */
    DMA_IRQPriorityConfig(IRQ_NO_DMA1_STREAM6, 15);
    DMA_IRQInterruptConfig(IRQ_NO_DMA1_STREAM6, ENABLE);
}

/* ============================================================
 * USART2 DMA send function
 * ============================================================
 */

static uint8_t USART2_DMA_Send(uint8_t *pTxBuffer, uint32_t Len)
{
    /*
     * Reject invalid requests.
     */
    if (pTxBuffer == 0 || Len == 0)
    {
        return DMA_TX_ERROR;
    }

    /*
     * If DMA is still sending the previous message, do not start a new one.
     *
     * This protects msg[] from being overwritten while DMA is using it.
     */
    if (usart2_dma_tx_busy == USART2_DMA_TX_BUSY)
    {
        return DMA_TX_BUSY;
    }

    /*
     * Mark DMA as busy before configuring the transfer.
     */
    usart2_dma_tx_busy = USART2_DMA_TX_BUSY;

    /*
     * DMA stream must be disabled before changing PAR, M0AR, or NDTR.
     */
    DMA_DisableStream(&usart2TxDmaHandle);

    /*
     * Clear previous transfer flags.
     */
    DMA_ClearFlags(&usart2TxDmaHandle);

    /*
     * Configure DMA addresses.
     *
     * Peripheral address:
     *   USART2 data register.
     *
     * Memory address:
     *   CSV string buffer.
     */
    DMA_ConfigAddresses(&usart2TxDmaHandle,
                        (uint32_t)&usart2Handle.pUSART->DR,
                        (uint32_t)pTxBuffer);

    /*
     * Number of data items.
     *
     * Because both memory and peripheral data size are BYTE,
     * Len is the number of bytes.
     */
    DMA_SetDataLength(&usart2TxDmaHandle, Len);

    /*
     * Enable USART2 DMA transmit request.
     *
     * USART2 will request DMA service whenever its transmit register is ready.
     */
    USART2->CR3 |= USART_CR3_DMAT;

    /*
     * Start DMA transfer.
     */
    DMA_EnableStream(&usart2TxDmaHandle);

    return DMA_TX_OK;
}

/* ============================================================
 * I2C1 GPIO initialization
 * ============================================================
 */

static void I2C1_GPIOInit(void)
{
    GPIO_Handle_t gpio;
    gpio.pGPIOx = GPIOB;

    /*
     * I2C pins use alternate function mode.
     *
     * PB6 -> I2C1_SCL
     * PB7 -> I2C1_SDA
     *
     * I2C requires open-drain output type.
     */
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

/* ============================================================
 * I2C1 peripheral initialization
 * ============================================================
 */

static void I2C1_Init(void)
{
    /*
     * Configure I2C1 as master for communication with LSM6DSO32.
     */
    i2c1Handle.pI2Cx = I2C1;
    i2c1Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    i2c1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    i2c1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    i2c1Handle.I2C_Config.I2C_DeviceAddress = 0x61;

    I2C_Init(&i2c1Handle);
}

/* ============================================================
 * USART2 GPIO initialization
 * ============================================================
 */

static void USART2_GPIOInit(void)
{
    GPIO_Handle_t gpio;
    gpio.pGPIOx = GPIOA;

    /*
     * PA2 -> USART2_TX
     * PA3 -> USART2_RX
     *
     * AF7 selects USART2 on these pins.
     */
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 7;
    gpio.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    GPIO_Init(&gpio);

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
    GPIO_Init(&gpio);
}

/* ============================================================
 * USART2 peripheral initialization
 * ============================================================
 */

static void USART2_Init(void)
{
    /*
     * USART2 is used for CSV data logging to the PC.
     */
    usart2Handle.pUSART = USART2;
    usart2Handle.USART_Config.Mode = USART_MODE_TXRX;
    usart2Handle.USART_Config.Baud = USART_STD_BAUD_115200;
    usart2Handle.USART_Config.NoOfStopBits = USART_STOPBITS_1;
    usart2Handle.USART_Config.WordLength = USART_WORDLEN_8BITS;
    usart2Handle.USART_Config.ParityControl = USART_PARITY_DISABLE;
    usart2Handle.USART_Config.HWFlowControl = USART_HW_FLOW_CTRL_NONE;

    USART_Init(&usart2Handle);
}

/* ============================================================
 * Debug GPIO initialization
 * ============================================================
 */

static void GPIODInit(void)
{
    GPIO_Handle_t gpiod;
    gpiod.pGPIOx = GPIOD;

    /*
     * Debug pins used with logic analyzer:
     *
     * PD12:
     *   High during IMU read.
     *
     * PD13:
     *   High during logging/processing section.
     */
    gpiod.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    gpiod.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
    gpiod.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    gpiod.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    gpiod.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    GPIO_Init(&gpiod);

    gpiod.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&gpiod);
}

/* ============================================================
 * TIM2 initialization
 * ============================================================
 */

static void TIM2_Init(void)
{
    /*
     * TIM2 generates update interrupt at approximately 100 Hz.
     *
     * With your current clock setup:
     *
     * Prescaler = 15999
     * AutoReload = 9
     *
     * Result:
     *   Period ≈ 10 ms
     */
    tim2Handle.pTIMx = TIM2;
    tim2Handle.TIM_Config.AutoReload = 9;
    tim2Handle.TIM_Config.Prescaler = 15999;

    TIM_Init(&tim2Handle);

    /*
     * Low priority timer interrupt.
     * ISR only sets sample_flag.
     */
    TIM_IRQPriorityConfig(IRQ_NO_TIM2, 15);
    TIM_EnableUpdateInterrupt(TIM2, IRQ_NO_TIM2);
}

/* ============================================================
 * TIM2 interrupt handler
 * ============================================================
 */

void TIM2_IRQHandler(void)
{
    /*
     * Check update interrupt flag.
     */
    if (tim2Handle.pTIMx->SR & TIM_SR_UIF)
    {
        /*
         * Clear update interrupt flag.
         */
        tim2Handle.pTIMx->SR &= ~TIM_SR_UIF;

        /*
         * Schedule one IMU update in the main loop.
         */
        sample_flag = 1;
    }
}

/* ============================================================
 * IMU raw read
 * ============================================================
 */

static void imu_read_raw(LSM6DSO32_t *imu)
{
    /*
     * Read accelerometer and gyroscope values from LSM6DSO32.
     *
     * This is currently blocking I2C.
     * Your timing measurements showed this is now the dominant runtime cost.
     */
    LSM6DSO32_ReadAccelG(imu, &accel_g);
    LSM6DSO32_ReadGyroDPS(imu, &gyro_dps);
}

/* ============================================================
 * IMU calibration
 * ============================================================
 */

static void imu_apply_calibration(void)
{
    /*
     * Convert accelerometer from g to mg and subtract bias.
     */
    a.x = accel_g.x * 1000 - AX_BIAS;
    a.y = accel_g.y * 1000 - AY_BIAS;
    a.z = accel_g.z * 1000 - AZ_BIAS;

    /*
     * Convert gyro from dps to mdps and subtract bias.
     *
     * The estimator converts mdps back to dps before angle integration.
     */
    g.x = gyro_dps.x * 1000 - GX_BIAS;
    g.y = gyro_dps.y * 1000 - GY_BIAS;
    g.z = gyro_dps.z * 1000 - GZ_BIAS;
}

/* ============================================================
 * IMU low-pass filtering
 * ============================================================
 */

static void imu_apply_filter(void)
{
    /*
     * Initialize filter output with first calibrated sample.
     *
     * This prevents a startup transient from zero.
     */
    if (!filter_initialized)
    {
        a_f.x = a.x;
        a_f.y = a.y;
        a_f.z = a.z;

        g_f.x = g.x;
        g_f.y = g.y;
        g_f.z = g.z;

        filter_initialized = 1;
        return;
    }

    /*
     * First-order IIR low-pass filter:
     *
     * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
     */
    a_f.x = ALPHA * a.x + (1.0f - ALPHA) * a_f.x;
    a_f.y = ALPHA * a.y + (1.0f - ALPHA) * a_f.y;
    a_f.z = ALPHA * a.z + (1.0f - ALPHA) * a_f.z;

    g_f.x = ALPHA * g.x + (1.0f - ALPHA) * g_f.x;
    g_f.y = ALPHA * g.y + (1.0f - ALPHA) * g_f.y;
    g_f.z = ALPHA * g.z + (1.0f - ALPHA) * g_f.z;
}

/* ============================================================
 * IMU logging
 * ============================================================
 */
static void imu_process(void)
{
    /*
     * Protect msg[] while DMA is transmitting.
     */
    if (usart2_dma_tx_busy == USART2_DMA_TX_BUSY)
    {
        return;
    }

    /*
     * Scale angles by 100 so they can be logged as integers.
     *
     * Example:
     *   12.34 deg -> 1234
     */
    int roll_kalman_i = (int)(IMU_Estimator_GetRollKalman() * 100);
    int pitch_kalman_i   = (int)(IMU_Estimator_GetPitchKalman() * 100);

    /*
     * CSV format:
     * counter,roll_comp,roll_kalman,roll_acc,roll_gyro
     */
    sprintf(msg,
            "%lu,%d,%d\r\n",
            sample_count++,
            roll_kalman_i,
            pitch_kalman_i);

    USART2_DMA_Send((uint8_t *)msg, strlen(msg));
}

/* ============================================================
 * Main application
 * ============================================================
 */

int main(void)
{
    LSM6DSO32_t imu;

    /*
     * Initialize low-level peripherals.
     */
    I2C1_GPIOInit();
    I2C1_Init();

    USART2_GPIOInit();
    USART2_Init();

    TIM2_Init();

    GPIODInit();

    /*
     * Configure USART2 TX DMA.
     *
     * This prepares DMA but does not start logging yet.
     */
    USART2_TX_DMA_Init();

    /*
     * Configure IMU driver object.
     */
    imu.address = LSM6DSO32_I2C_ADDR;
    imu.hi2c = &i2c1Handle;

    /*
     * Initialize the IMU sensor.
     */
    if (!LSM6DSO32_Init(&imu))
    {
        sprintf(msg, "LSM6DSO32 init failed\r\n");
        USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));

        while (1);
    }

    /*
     * Initialize estimator before the timer starts.
     *
     * This guarantees that the first sampled frame starts from a known state.
     */
    IMU_Estimator_Init();

    /*
     * Print startup messages before enabling TIM2.
     *
     * Important:
     * Once TIM2 starts, DMA logging may begin.
     * Do not mix blocking USART startup messages with DMA log output.
     */
    sprintf(msg, "LSM6DSO32 init OK\r\n");
    USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));

    sprintf(msg, "START\r\n");
    USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));

    sprintf(msg,
            "counter,roll,pitch\r\n");
    USART_SendData(&usart2Handle, (uint8_t *)msg, strlen(msg));

    /*
     * Start 100 Hz sampling only after initialization and CSV header.
     */
    TIM_PeripheralControl(TIM2, ENABLE);

    while (1)
    {
        /*
         * Run one IMU processing step whenever TIM2 schedules it.
         */
        if (sample_flag)
        {
            sample_flag = 0;

            /*
             * Measure IMU read time with logic analyzer.
             */
            GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_12, SET);
            imu_read_raw(&imu);
            GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_12, RESET);

            /*
             * Convert raw sensor data into usable filtered values.
             */
            imu_apply_calibration();
            imu_apply_filter();

            /*
             * Update roll/pitch estimator using filtered data.
             */
            IMU_Estimator_Update(a_f.x,
                                 a_f.y,
                                 a_f.z,
                                 g_f.x,
                                 g_f.y,
                                 DT);

            /*
             * Measure logging/processing launch time.
             */
            GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_13, SET);
            imu_process();
            GPIO_WriteToOutputPin(GPIOD, GPIO_PIN_NO_13, RESET);
        }
    }
}

/* ============================================================
 * DMA1 Stream6 interrupt handler
 * ============================================================
 */

void DMA1_Stream6_IRQHandler(void)
{
    /*
     * Transfer complete flag for DMA1 Stream 6.
     *
     * This means DMA has moved all bytes from msg[] into USART2->DR.
     */
    if (DMA_GetFlagStatus(&usart2TxDmaHandle, DMA_FLAG_TCIF2_6) == FLAG_SET)
    {
        /*
         * Clear all Stream 6 DMA flags.
         */
        DMA_ClearFlags(&usart2TxDmaHandle);

        /*
         * Disable stream after normal-mode transfer.
         */
        DMA_DisableStream(&usart2TxDmaHandle);

        /*
         * Disable USART2 DMA transmit request.
         */
        USART2->CR3 &= ~USART_CR3_DMAT;

        /*
         * Release msg[] buffer.
         */
        usart2_dma_tx_busy = USART2_DMA_TX_IDLE;

        return;
    }

    /*
     * Transfer error handling.
     */
    if (DMA_GetFlagStatus(&usart2TxDmaHandle, DMA_FLAG_TEIF2_6) == FLAG_SET)
    {
        DMA_ClearFlags(&usart2TxDmaHandle);
        DMA_DisableStream(&usart2TxDmaHandle);
        USART2->CR3 &= ~USART_CR3_DMAT;

        /*
         * Even after an error, release the buffer so the system can continue.
         */
        usart2_dma_tx_busy = USART2_DMA_TX_IDLE;
    }
}
