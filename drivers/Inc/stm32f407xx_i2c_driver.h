/*
 * stm32f407xx_i2c_driver.h
 *
 *  Created on: Feb 11, 2026
 *      Author: Rasoul
 */

#ifndef INC_STM32F407XX_I2C_DRIVER_H_
#define INC_STM32F407XX_I2C_DRIVER_H_

#include "stm32f407xx.h"
#include <stdint.h>

/*
 * Configuration structure for I2Cx peripheral
 */
typedef struct
{
    uint32_t I2C_SCLSpeed;
    uint8_t  I2C_DeviceAddress;
    uint8_t  I2C_AckControl;
    uint8_t  I2C_FMDutyCycle;
} I2C_Config_t;

/*
 * Handle structure for I2Cx peripheral
 */
typedef struct
{
    I2C_RegDef_t *pI2Cx;
    I2C_Config_t  I2C_Config;

    uint8_t  *pTxBuffer;
    uint8_t  *pRxBuffer;
    uint32_t  TxLen;
    uint32_t  RxLen;
    uint32_t  RxSize;

    uint8_t   TxRxState;
    uint8_t   DevAddr;
    uint8_t   Sr;
} I2C_Handle_t;

/*
 * Return status for blocking APIs
 */
typedef enum
{
    I2C_OK = 0,
    I2C_ERROR_AF,
    I2C_ERROR_TIMEOUT,
    I2C_ERROR_BUSY
} I2C_Status_t;

/*
 * I2C application state
 */
#define I2C_READY                       0U
#define I2C_BUSY_IN_RX                  1U
#define I2C_BUSY_IN_TX                  2U

/*
 * I2C SCL speed
 */
#define I2C_SCL_SPEED_SM               100000U
#define I2C_SCL_SPEED_FM_200K          200000U
#define I2C_SCL_SPEED_FM_400K          400000U

/* Backward compatibility */
#define I2C_SCL_SPEED_FM2K             I2C_SCL_SPEED_FM_200K
#define I2C_SCL_SPEED_FM4K             I2C_SCL_SPEED_FM_400K

/*
 * I2C ACK control
 */
#define I2C_ACK_ENABLE                 1U
#define I2C_ACK_DISABLE                0U

/*
 * I2C Fast Mode duty cycle
 */
#define I2C_FM_DUTY_2                  0U
#define I2C_FM_DUTY_16_9               1U

/*
 * Application events and errors for callback
 */
#define I2C_APP_EV_TX_CMPLT            0U
#define I2C_APP_EV_RX_CMPLT            1U
#define I2C_APP_EV_STOP                2U
#define I2C_APP_EV_DATA_REQ            3U
#define I2C_APP_EV_DATA_RCV            4U

#define I2C_APP_ERROR_BERR             5U
#define I2C_APP_ERROR_ARLO             6U
#define I2C_APP_ERROR_AF               7U
#define I2C_APP_ERROR_OVR              8U
#define I2C_APP_ERROR_TIMEOUT          9U

/* Optional backward-compatible aliases */
#define I2C_EV_TX_CMPLT                I2C_APP_EV_TX_CMPLT
#define I2C_EV_RX_CMPLT                I2C_APP_EV_RX_CMPLT
#define I2C_EV_STOP                    I2C_APP_EV_STOP
#define I2C_EV_DATA_REQ                I2C_APP_EV_DATA_REQ
#define I2C_EV_DATA_RCV                I2C_APP_EV_DATA_RCV

/******************************************************************
 *                         I2C APIs Prototype
 ******************************************************************/

/*
 * Peripheral clock setup
 */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);

/*
 * Init and De-Init
 */
void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_RegDef_t *pI2Cx);

/*
 * Data send and receive
 */
I2C_Status_t I2C_MasterSendData(I2C_Handle_t *pI2CHandle,
                                uint8_t *pBuffer,
                                uint32_t Len,
                                uint8_t SlaveAddr,
                                uint8_t Sr);

I2C_Status_t I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle,
                                   uint8_t *pBuffer,
                                   uint32_t Len,
                                   uint8_t SlaveAddr,
                                   uint8_t Sr);

uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,
                             uint8_t *pBuffer,
                             uint32_t Len,
                             uint8_t SlaveAddr,
                             uint8_t Sr);

uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,
                                uint8_t *pBuffer,
                                uint32_t Len,
                                uint8_t SlaveAddr,
                                uint8_t Sr);

void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data);
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C);

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2C, uint8_t EnOrDi);

/*
 * IRQ configuration and ISR handling
 * This helper enables/disables both EV and ER IRQ lines for the selected I2C peripheral.
 */
void I2C_IRQInterruptConfig(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle);
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle);

/*
 * Other peripheral APIs
 */
void I2C_PeriControl(I2C_Handle_t *pI2CHandle, uint8_t EnOrDi);
uint8_t I2C_GetFlagStatus_SR1(I2C_Handle_t *pI2CHandle, uint32_t Flag);
uint8_t I2C_GetFlagStatus_SR2(I2C_Handle_t *pI2CHandle, uint32_t Flag);

/*
 * Application callback
 */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEvent);

/******************************************************************
 *                    I2C Bit Position Definition
 ******************************************************************/

/* CR1 */
#define I2C_CR1_PE_POS                   0U
#define I2C_CR1_PE                       (1U << I2C_CR1_PE_POS)

#define I2C_CR1_SMBUS_POS                1U
#define I2C_CR1_SMBTYPE_POS              3U
#define I2C_CR1_ENARP_POS                4U
#define I2C_CR1_ENPEC_POS                5U
#define I2C_CR1_ENGC_POS                 6U
#define I2C_CR1_NOSTRETCH_POS            7U

#define I2C_CR1_START_POS                8U
#define I2C_CR1_START                    (1U << I2C_CR1_START_POS)

#define I2C_CR1_STOP_POS                 9U
#define I2C_CR1_STOP                     (1U << I2C_CR1_STOP_POS)

#define I2C_CR1_ACK_POS                  10U
#define I2C_CR1_ACK                      (1U << I2C_CR1_ACK_POS)

#define I2C_CR1_POS_POS                  11U
#define I2C_CR1_POS                      (1U << I2C_CR1_POS_POS)

#define I2C_CR1_PEC_POS                  12U
#define I2C_CR1_ALERT_POS                13U
#define I2C_CR1_SWRST_POS                15U

/* CR2 */
#define I2C_CR2_FREQ_POS                 0U
#define I2C_CR2_ITERREN_POS              8U
#define I2C_CR2_ITERREN                  (1U << I2C_CR2_ITERREN_POS)

#define I2C_CR2_ITEVTEN_POS              9U
#define I2C_CR2_ITEVTEN                  (1U << I2C_CR2_ITEVTEN_POS)

#define I2C_CR2_ITBUFEN_POS              10U
#define I2C_CR2_ITBUFEN                  (1U << I2C_CR2_ITBUFEN_POS)

#define I2C_CR2_DMAEN_POS                11U
#define I2C_CR2_LAST_POS                 12U

/* SR1 bit positions */
#define I2C_SR1_SB_POS                   0U
#define I2C_SR1_ADDR_POS                 1U
#define I2C_SR1_BTF_POS                  2U
#define I2C_SR1_ADD10_POS                3U
#define I2C_SR1_STOPF_POS                4U
#define I2C_SR1_RXNE_POS                 6U
#define I2C_SR1_TXE_POS                  7U
#define I2C_SR1_BERR_POS                 8U
#define I2C_SR1_ARLO_POS                 9U
#define I2C_SR1_AF_POS                   10U
#define I2C_SR1_OVR_POS                  11U
#define I2C_SR1_PECERR_POS               12U
#define I2C_SR1_TIMEOUT_POS              14U
#define I2C_SR1_SMBALERT_POS             15U

/* SR1 flag masks */
#define I2C_FLAG_SR1_SB                  (1U << I2C_SR1_SB_POS)
#define I2C_FLAG_SR1_ADDR                (1U << I2C_SR1_ADDR_POS)
#define I2C_FLAG_SR1_BTF                 (1U << I2C_SR1_BTF_POS)
#define I2C_FLAG_SR1_ADD10               (1U << I2C_SR1_ADD10_POS)
#define I2C_FLAG_SR1_STOPF               (1U << I2C_SR1_STOPF_POS)
#define I2C_FLAG_SR1_RXNE                (1U << I2C_SR1_RXNE_POS)
#define I2C_FLAG_SR1_TXE                 (1U << I2C_SR1_TXE_POS)
#define I2C_FLAG_SR1_BERR                (1U << I2C_SR1_BERR_POS)
#define I2C_FLAG_SR1_ARLO                (1U << I2C_SR1_ARLO_POS)
#define I2C_FLAG_SR1_AF                  (1U << I2C_SR1_AF_POS)
#define I2C_FLAG_SR1_OVR                 (1U << I2C_SR1_OVR_POS)
#define I2C_FLAG_SR1_PECERR              (1U << I2C_SR1_PECERR_POS)
#define I2C_FLAG_SR1_TIMEOUT             (1U << I2C_SR1_TIMEOUT_POS)
#define I2C_FLAG_SR1_SMBALERT            (1U << I2C_SR1_SMBALERT_POS)

/* Backward compatibility */
#define I2C_FLAG_SR1_TxE                 I2C_FLAG_SR1_TXE
#define I2C_FLAG_SR1_RxNE                I2C_FLAG_SR1_RXNE

/* SR2 bit positions */
#define I2C_SR2_MSL_POS                  0U
#define I2C_SR2_BUSY_POS                 1U
#define I2C_SR2_TRA_POS                  2U
#define I2C_SR2_GENCALL_POS              4U
#define I2C_SR2_SMBDEFAULT_POS           5U
#define I2C_SR2_SMBHOST_POS              6U
#define I2C_SR2_DUALF_POS                7U
#define I2C_SR2_PEC_POS                  8U

/* SR2 flag masks */
#define I2C_FLAG_SR2_MSL                 (1U << I2C_SR2_MSL_POS)
#define I2C_FLAG_SR2_BUSY                (1U << I2C_SR2_BUSY_POS)
#define I2C_FLAG_SR2_TRA                 (1U << I2C_SR2_TRA_POS)
#define I2C_FLAG_SR2_GENCALL             (1U << I2C_SR2_GENCALL_POS)
#define I2C_FLAG_SR2_SMBDEFAULT          (1U << I2C_SR2_SMBDEFAULT_POS)
#define I2C_FLAG_SR2_SMBHOST             (1U << I2C_SR2_SMBHOST_POS)
#define I2C_FLAG_SR2_DUALF               (1U << I2C_SR2_DUALF_POS)
#define I2C_FLAG_SR2_PEC                 (1U << I2C_SR2_PEC_POS)

/* CCR */
#define I2C_CCR_CCR_POS                  0U
#define I2C_CCR_DUTY_POS                 14U
#define I2C_CCR_F_S_POS                  15U

/* OAR1 */
#define I2C_OAR1_ADD_POS                 1U

#endif /* INC_STM32F407XX_I2C_DRIVER_H_ */
