/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: Feb 11, 2026
 *      Author: Rasoul
 */

#include "stm32f407xx_i2c_driver.h"
#include "stm32f407xx_rcc_driver.h"
#include <stddef.h>

#define I2C_TIMEOUT  500000U

/* -------------------------------------------------------------------------- */
/* Static helper functions                                                    */
/* -------------------------------------------------------------------------- */
static void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pI2Cx->CR1 |= I2C_CR1_ACK;
    }
    else
    {
        pI2Cx->CR1 &= ~I2C_CR1_ACK;
    }
}

static int I2C_WaitWhileBusy(I2C_Handle_t *pI2CHandle)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (I2C_GetFlagStatus_SR2(pI2CHandle, I2C_FLAG_SR2_BUSY))
    {
        if (timeout-- == 0U)
        {
            return -1;
        }
    }

    return 0;
}

static void I2C_RestoreAckConfig(I2C_Handle_t *pI2CHandle)
{
    I2C_ManageAcking(pI2CHandle->pI2Cx, pI2CHandle->I2C_Config.I2C_AckControl);
}

static void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
    pI2CHandle->pI2Cx->CR2 &= ~I2C_CR2_ITBUFEN;
    pI2CHandle->pI2Cx->CR2 &= ~I2C_CR2_ITEVTEN;
    pI2CHandle->pI2Cx->CR2 &= ~I2C_CR2_ITERREN;

    pI2CHandle->pTxBuffer = NULL;
    pI2CHandle->TxLen     = 0U;
    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->DevAddr   = 0U;
    pI2CHandle->Sr        = I2C_DISABLE_SR;
}

static void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
    pI2CHandle->pI2Cx->CR2 &= ~I2C_CR2_ITBUFEN;
    pI2CHandle->pI2Cx->CR2 &= ~I2C_CR2_ITEVTEN;
    pI2CHandle->pI2Cx->CR2 &= ~I2C_CR2_ITERREN;

    pI2CHandle->pRxBuffer = NULL;
    pI2CHandle->RxLen     = 0U;
    pI2CHandle->RxSize    = 0U;
    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->DevAddr   = 0U;
    pI2CHandle->Sr        = I2C_DISABLE_SR;

    pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
    I2C_RestoreAckConfig(pI2CHandle);
}

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle)
{
    if ((pI2CHandle->TxRxState == I2C_BUSY_IN_TX) && (pI2CHandle->TxLen > 0U))
    {
        pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);
        pI2CHandle->pTxBuffer++;
        pI2CHandle->TxLen--;
    }
}

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle)
{
    if (pI2CHandle->TxRxState != I2C_BUSY_IN_RX)
    {
        return;
    }

    if (pI2CHandle->RxSize == 1U)
    {
        if (pI2CHandle->RxLen == 1U)
        {
            *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
            pI2CHandle->RxLen = 0U;

            I2C_CloseReceiveData(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_RX_CMPLT);
        }
    }
    else if (pI2CHandle->RxSize > 2U)
    {
        if (pI2CHandle->RxLen > 3U)
        {
            *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
            pI2CHandle->pRxBuffer++;
            pI2CHandle->RxLen--;
        }
    }
    else
    {
        /* RxSize == 2U is handled in BTF event */
    }
}

static void I2C_MasterHandleBTFInterrupt(I2C_Handle_t *pI2CHandle)
{
    if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
    {
        if ((pI2CHandle->TxLen == 0U) && (pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_TXE))
        {
            if (pI2CHandle->Sr == I2C_DISABLE_SR)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            }

            I2C_CloseSendData(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_TX_CMPLT);
        }
    }
    else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        if (pI2CHandle->RxSize == 2U)
        {
            if (pI2CHandle->RxLen == 2U)
            {
                if (pI2CHandle->Sr == I2C_DISABLE_SR)
                {
                    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                }

                *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
                pI2CHandle->pRxBuffer++;
                pI2CHandle->RxLen--;

                *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
                pI2CHandle->pRxBuffer++;
                pI2CHandle->RxLen--;

                I2C_CloseReceiveData(pI2CHandle);
                I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_RX_CMPLT);
            }
        }
        else if (pI2CHandle->RxSize > 2U)
        {
            if (pI2CHandle->RxLen == 3U)
            {
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

                *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
                pI2CHandle->pRxBuffer++;
                pI2CHandle->RxLen--;
            }
            else if (pI2CHandle->RxLen == 2U)
            {
                if (pI2CHandle->Sr == I2C_DISABLE_SR)
                {
                    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                }

                *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
                pI2CHandle->pRxBuffer++;
                pI2CHandle->RxLen--;

                *(pI2CHandle->pRxBuffer) = (uint8_t)pI2CHandle->pI2Cx->DR;
                pI2CHandle->pRxBuffer++;
                pI2CHandle->RxLen--;

                I2C_CloseReceiveData(pI2CHandle);
                I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_RX_CMPLT);
            }
        }
    }
}

static void I2C_AbortTransfer(I2C_Handle_t *pI2CHandle)
{
    if ((pI2CHandle->pI2Cx->SR2 & I2C_FLAG_SR2_MSL) != 0U)
    {
        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
    }

    if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
    {
        I2C_CloseSendData(pI2CHandle);
    }
    else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        I2C_CloseReceiveData(pI2CHandle);
    }
}

static void I2C_NVIC_EnableIRQ(uint8_t IRQNumber)
{
    if (IRQNumber <= 31U)
    {
        *NVIC_ISER0 |= (1U << IRQNumber);
    }
    else if (IRQNumber <= 63U)
    {
        *NVIC_ISER1 |= (1U << (IRQNumber % 32U));
    }
    else if (IRQNumber <= 95U)
    {
        *NVIC_ISER2 |= (1U << (IRQNumber % 64U));
    }
    else
    {
        *NVIC_ISER3 |= (1U << (IRQNumber % 96U));
    }
}

static void I2C_NVIC_DisableIRQ(uint8_t IRQNumber)
{
    if (IRQNumber <= 31U)
    {
        *NVIC_ICER0 |= (1U << IRQNumber);
    }
    else if (IRQNumber <= 63U)
    {
        *NVIC_ICER1 |= (1U << (IRQNumber % 32U));
    }
    else if (IRQNumber <= 95U)
    {
        *NVIC_ICER2 |= (1U << (IRQNumber % 64U));
    }
    else
    {
        *NVIC_ICER3 |= (1U << (IRQNumber % 96U));
    }
}

/* -------------------------------------------------------------------------- */
/* Peripheral clock control                                                   */
/* -------------------------------------------------------------------------- */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        if (pI2Cx == I2C1)
        {
            I2C1_PCLK_EN();
        }
        else if (pI2Cx == I2C2)
        {
            I2C2_PCLK_EN();
        }
        else if (pI2Cx == I2C3)
        {
            I2C3_PCLK_EN();
        }
    }
    else
    {
        if (pI2Cx == I2C1)
        {
            I2C1_PCLK_DI();
        }
        else if (pI2Cx == I2C2)
        {
            I2C2_PCLK_DI();
        }
        else if (pI2Cx == I2C3)
        {
            I2C3_PCLK_DI();
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Init / De-init                                                             */
/* -------------------------------------------------------------------------- */
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
    uint16_t ccr = 0U;
    uint32_t pclk1Hz;
    uint32_t freqMHz;
    uint32_t fsclHz;

    pclk1Hz = RCC_GetPCLK1Value();
    freqMHz = pclk1Hz / 1000000U;
    fsclHz  = pI2CHandle->I2C_Config.I2C_SCLSpeed;

    I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

    /* Disable peripheral before configuration */
    pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_PE;

    /* Configure ACK */
    I2C_ManageAcking(pI2CHandle->pI2Cx, pI2CHandle->I2C_Config.I2C_AckControl);

    /* Configure CR2.FREQ */
    pI2CHandle->pI2Cx->CR2 &= ~0x3FU;
    pI2CHandle->pI2Cx->CR2 |= (freqMHz & 0x3FU);

    /* Configure CCR and TRISE */
    pI2CHandle->pI2Cx->CCR = 0U;

    if (fsclHz <= I2C_SCL_SPEED_SM)
    {
        /* Standard mode */
        pI2CHandle->pI2Cx->CCR &= ~(1U << I2C_CCR_F_S_POS);

        ccr = (uint16_t)(pclk1Hz / (2U * fsclHz));
        if (ccr < 4U)
        {
            ccr = 4U;
        }

        pI2CHandle->pI2Cx->CCR |= (ccr & 0x0FFFU);
        pI2CHandle->pI2Cx->TRISE = (freqMHz + 1U) & 0x3FU;
    }
    else
    {
        /* Fast mode */
        pI2CHandle->pI2Cx->CCR |= (1U << I2C_CCR_F_S_POS);

        if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_16_9)
        {
            pI2CHandle->pI2Cx->CCR |= (1U << I2C_CCR_DUTY_POS);
            ccr = (uint16_t)(pclk1Hz / (25U * fsclHz));
        }
        else
        {
            pI2CHandle->pI2Cx->CCR &= ~(1U << I2C_CCR_DUTY_POS);
            ccr = (uint16_t)(pclk1Hz / (3U * fsclHz));
        }

        if (ccr == 0U)
        {
            ccr = 1U;
        }

        pI2CHandle->pI2Cx->CCR |= (ccr & 0x0FFFU);
        pI2CHandle->pI2Cx->TRISE = (((freqMHz * 300U) / 1000U) + 1U) & 0x3FU;
    }

    /* Configure own address */
    pI2CHandle->pI2Cx->OAR1 = 0U;
    pI2CHandle->pI2Cx->OAR1 |= ((uint32_t)pI2CHandle->I2C_Config.I2C_DeviceAddress << I2C_OAR1_ADD_POS);
    pI2CHandle->pI2Cx->OAR1 |= (1U << 14);

    /* Enable peripheral */
    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_PE;
}

void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
    if (pI2Cx == I2C1)
    {
        I2C1_REG_RESET();
    }
    else if (pI2Cx == I2C2)
    {
        I2C2_REG_RESET();
    }
    else if (pI2Cx == I2C3)
    {
        I2C3_REG_RESET();
    }
}

void I2C_PeriControl(I2C_Handle_t *pI2CHandle, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_PE;
    }
    else
    {
        pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_PE;
    }
}

/* -------------------------------------------------------------------------- */
/* Blocking master transmit                                                   */
/* -------------------------------------------------------------------------- */
I2C_Status_t I2C_MasterSendData(I2C_Handle_t *pI2CHandle,
                                uint8_t *pBuffer,
                                uint32_t Len,
                                uint8_t SlaveAddr)
{
    uint32_t timeout;

    if (Len == 0U)
    {
        return I2C_OK;
    }

    if (I2C_WaitWhileBusy(pI2CHandle) < 0)
    {
        return I2C_ERROR_BUSY;
    }

    /* Generate START */
    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_START;

    /* Wait for SB */
    timeout = I2C_TIMEOUT;
    while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_SB))
    {
        if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
        {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_AF;
        }

        if (timeout-- == 0U)
        {
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_TIMEOUT;
        }
    }

    /* Send slave address + W */
    pI2CHandle->pI2Cx->DR = (uint8_t)(SlaveAddr << 1);

    /* Wait for ADDR */
    timeout = I2C_TIMEOUT;
    while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_ADDR))
    {
        if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
        {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_AF;
        }

        if (timeout-- == 0U)
        {
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_TIMEOUT;
        }
    }

    /* Clear ADDR */
    (void)pI2CHandle->pI2Cx->SR1;
    (void)pI2CHandle->pI2Cx->SR2;

    /* Send data */
    while (Len > 0U)
    {
        timeout = I2C_TIMEOUT;
        while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_TXE))
        {
            if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
            {
                pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                return I2C_ERROR_AF;
            }

            if (timeout-- == 0U)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                return I2C_ERROR_TIMEOUT;
            }
        }

        pI2CHandle->pI2Cx->DR = *pBuffer;
        pBuffer++;
        Len--;
    }

    /* Wait for TXE */
    timeout = I2C_TIMEOUT;
    while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_TXE))
    {
        if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
        {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_AF;
        }

        if (timeout-- == 0U)
        {
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_TIMEOUT;
        }
    }

    /* Wait for BTF */
    timeout = I2C_TIMEOUT;
    while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_BTF))
    {
        if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
        {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_AF;
        }

        if (timeout-- == 0U)
        {
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            return I2C_ERROR_TIMEOUT;
        }
    }

    /* Generate STOP */
    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;

    return I2C_OK;
}

/* -------------------------------------------------------------------------- */
/* Blocking master receive                                                    */
/* -------------------------------------------------------------------------- */
I2C_Status_t I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle,
                                   uint8_t *pBuffer,
                                   uint32_t Len,
                                   uint8_t SlaveAddr)
{
    uint32_t timeout;

    if (Len == 0U)
    {
        return I2C_OK;
    }

    if (I2C_WaitWhileBusy(pI2CHandle) < 0)
    {
        return I2C_ERROR_BUSY;
    }

    /* Generate START */
    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_START;

    /* Wait for SB */
    timeout = I2C_TIMEOUT;
    while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_SB))
    {
        if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
        {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            I2C_RestoreAckConfig(pI2CHandle);
            pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
            return I2C_ERROR_AF;
        }

        if (timeout-- == 0U)
        {
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            I2C_RestoreAckConfig(pI2CHandle);
            pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
            return I2C_ERROR_TIMEOUT;
        }
    }

    /* Send slave address + R */
    pI2CHandle->pI2Cx->DR = (uint8_t)((SlaveAddr << 1) | 1U);

    /* Wait for ADDR */
    timeout = I2C_TIMEOUT;
    while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_ADDR))
    {
        if (I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_AF))
        {
            pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            I2C_RestoreAckConfig(pI2CHandle);
            pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
            return I2C_ERROR_AF;
        }

        if (timeout-- == 0U)
        {
            pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
            I2C_RestoreAckConfig(pI2CHandle);
            pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
            return I2C_ERROR_TIMEOUT;
        }
    }

    if (Len == 1U)
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

        (void)pI2CHandle->pI2Cx->SR1;
        (void)pI2CHandle->pI2Cx->SR2;

        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;

        timeout = I2C_TIMEOUT;
        while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_RXNE))
        {
            if (timeout-- == 0U)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                I2C_RestoreAckConfig(pI2CHandle);
                pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
                return I2C_ERROR_TIMEOUT;
            }
        }

        *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
    }
    else if (Len == 2U)
    {
        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_POS;
        I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

        (void)pI2CHandle->pI2Cx->SR1;
        (void)pI2CHandle->pI2Cx->SR2;

        timeout = I2C_TIMEOUT;
        while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_BTF))
        {
            if (timeout-- == 0U)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                I2C_RestoreAckConfig(pI2CHandle);
                pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
                return I2C_ERROR_TIMEOUT;
            }
        }

        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;

        *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
        pBuffer++;

        *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
    }
    else
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);

        (void)pI2CHandle->pI2Cx->SR1;
        (void)pI2CHandle->pI2Cx->SR2;

        while (Len > 3U)
        {
            timeout = I2C_TIMEOUT;
            while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_RXNE))
            {
                if (timeout-- == 0U)
                {
                    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                    I2C_RestoreAckConfig(pI2CHandle);
                    pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
                    return I2C_ERROR_TIMEOUT;
                }
            }

            *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
            pBuffer++;
            Len--;
        }

        timeout = I2C_TIMEOUT;
        while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_BTF))
        {
            if (timeout-- == 0U)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                I2C_RestoreAckConfig(pI2CHandle);
                pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
                return I2C_ERROR_TIMEOUT;
            }
        }

        I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
        *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
        pBuffer++;
        Len--;

        timeout = I2C_TIMEOUT;
        while (!I2C_GetFlagStatus_SR1(pI2CHandle, I2C_FLAG_SR1_BTF))
        {
            if (timeout-- == 0U)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                I2C_RestoreAckConfig(pI2CHandle);
                pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;
                return I2C_ERROR_TIMEOUT;
            }
        }

        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;

        *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
        pBuffer++;

        *pBuffer = (uint8_t)pI2CHandle->pI2Cx->DR;
    }

    I2C_RestoreAckConfig(pI2CHandle);
    pI2CHandle->pI2Cx->CR1 &= ~I2C_CR1_POS;

    return I2C_OK;
}

/* -------------------------------------------------------------------------- */
/* Interrupt-based master transmit                                            */
/* -------------------------------------------------------------------------- */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,
                             uint8_t *pBuffer,
                             uint32_t Len,
                             uint8_t SlaveAddr,
                             uint8_t Sr)
{
    uint8_t busystate = pI2CHandle->TxRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
    {
        pI2CHandle->pTxBuffer = pBuffer;
        pI2CHandle->TxLen     = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
        pI2CHandle->DevAddr   = SlaveAddr;
        pI2CHandle->Sr        = Sr;

        pI2CHandle->pI2Cx->CR2 |= I2C_CR2_ITBUFEN;
        pI2CHandle->pI2Cx->CR2 |= I2C_CR2_ITEVTEN;
        pI2CHandle->pI2Cx->CR2 |= I2C_CR2_ITERREN;

        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_START;
    }

    return busystate;
}

/* -------------------------------------------------------------------------- */
/* Interrupt-based master receive                                             */
/* -------------------------------------------------------------------------- */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,
                                uint8_t *pBuffer,
                                uint32_t Len,
                                uint8_t SlaveAddr,
                                uint8_t Sr)
{
    uint8_t busystate = pI2CHandle->TxRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
    {
        pI2CHandle->pRxBuffer = pBuffer;
        pI2CHandle->RxLen     = Len;
        pI2CHandle->RxSize    = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
        pI2CHandle->DevAddr   = SlaveAddr;
        pI2CHandle->Sr        = Sr;

        pI2CHandle->pI2Cx->CR2 |= I2C_CR2_ITBUFEN;
        pI2CHandle->pI2Cx->CR2 |= I2C_CR2_ITEVTEN;
        pI2CHandle->pI2Cx->CR2 |= I2C_CR2_ITERREN;

        pI2CHandle->pI2Cx->CR1 |= I2C_CR1_START;
    }

    return busystate;
}

/* -------------------------------------------------------------------------- */
/* IRQ configuration                                                          */
/* -------------------------------------------------------------------------- */
void I2C_IRQInterruptConfig(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    uint8_t evIRQ = 0U;
    uint8_t erIRQ = 0U;

    if (pI2Cx == I2C1)
    {
        evIRQ = IRQ_NO_I2C1_EV;
        erIRQ = IRQ_NO_I2C1_ER;
    }
    else if (pI2Cx == I2C2)
    {
        evIRQ = IRQ_NO_I2C2_EV;
        erIRQ = IRQ_NO_I2C2_ER;
    }
    else if (pI2Cx == I2C3)
    {
        evIRQ = IRQ_NO_I2C3_EV;
        erIRQ = IRQ_NO_I2C3_ER;
    }
    else
    {
        return;
    }

    if (EnOrDi == ENABLE)
    {
        I2C_NVIC_EnableIRQ(evIRQ);
        I2C_NVIC_EnableIRQ(erIRQ);
    }
    else
    {
        I2C_NVIC_DisableIRQ(evIRQ);
        I2C_NVIC_DisableIRQ(erIRQ);
    }
}

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
{
    uint8_t iprx = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;
    uint8_t shift_amount = (uint8_t)((8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED));

    NVIC_PR_BASE_ADDR[iprx] &= ~(0xFU << shift_amount);
    NVIC_PR_BASE_ADDR[iprx] |= ((uint32_t)(IRQPriority & 0x0FU) << shift_amount);
}

/* -------------------------------------------------------------------------- */
/* Event IRQ handler                                                          */
/* -------------------------------------------------------------------------- */
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    uint32_t temp1;
    uint32_t temp2;
    uint32_t temp3;

    temp1 = pI2CHandle->pI2Cx->CR2 & I2C_CR2_ITEVTEN;
    temp2 = pI2CHandle->pI2Cx->CR2 & I2C_CR2_ITBUFEN;

    /* 1. SB event */
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_SB;
    if ((temp1 != 0U) && (temp3 != 0U))
    {
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
        {
            pI2CHandle->pI2Cx->DR = (uint8_t)(pI2CHandle->DevAddr << 1);
        }
        else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            pI2CHandle->pI2Cx->DR = (uint8_t)((pI2CHandle->DevAddr << 1) | 1U);
        }
    }

    /* 2. ADDR event */
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_ADDR;
    if ((temp1 != 0U) && (temp3 != 0U))
    {
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            if (pI2CHandle->RxSize == 1U)
            {
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

                (void)pI2CHandle->pI2Cx->SR1;
                (void)pI2CHandle->pI2Cx->SR2;

                if (pI2CHandle->Sr == I2C_DISABLE_SR)
                {
                    pI2CHandle->pI2Cx->CR1 |= I2C_CR1_STOP;
                }
            }
            else if (pI2CHandle->RxSize == 2U)
            {
                pI2CHandle->pI2Cx->CR1 |= I2C_CR1_POS;
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

                (void)pI2CHandle->pI2Cx->SR1;
                (void)pI2CHandle->pI2Cx->SR2;
            }
            else
            {
                (void)pI2CHandle->pI2Cx->SR1;
                (void)pI2CHandle->pI2Cx->SR2;
            }
        }
        else
        {
            (void)pI2CHandle->pI2Cx->SR1;
            (void)pI2CHandle->pI2Cx->SR2;
        }
    }

    /* 3. BTF event */
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_BTF;
    if ((temp1 != 0U) && (temp3 != 0U))
    {
        if ((pI2CHandle->pI2Cx->SR2 & I2C_FLAG_SR2_MSL) != 0U)
        {
            I2C_MasterHandleBTFInterrupt(pI2CHandle);
        }
    }

    /* 4. STOPF event - valid in slave mode */
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_STOPF;
    if ((temp1 != 0U) && (temp3 != 0U))
    {
        /* Clear STOPF: read SR1, then write CR1 */
        (void)pI2CHandle->pI2Cx->SR1;
        pI2CHandle->pI2Cx->CR1 = pI2CHandle->pI2Cx->CR1;

        I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_STOP);
    }

    /* 5. TXE event */
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_TXE;
    if ((temp1 != 0U) && (temp2 != 0U) && (temp3 != 0U))
    {
        if ((pI2CHandle->pI2Cx->SR2 & I2C_FLAG_SR2_MSL) != 0U)
        {
            I2C_MasterHandleTXEInterrupt(pI2CHandle);
        }
        else
        {
            if ((pI2CHandle->pI2Cx->SR2 & I2C_FLAG_SR2_TRA) != 0U)
            {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_DATA_REQ);
            }
        }
    }

    /* 6. RXNE event */
    temp3 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_RXNE;
    if ((temp1 != 0U) && (temp2 != 0U) && (temp3 != 0U))
    {
        if ((pI2CHandle->pI2Cx->SR2 & I2C_FLAG_SR2_MSL) != 0U)
        {
            I2C_MasterHandleRXNEInterrupt(pI2CHandle);
        }
        else
        {
            if ((pI2CHandle->pI2Cx->SR2 & I2C_FLAG_SR2_TRA) == 0U)
            {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_EV_DATA_RCV);
            }
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Error IRQ handler                                                          */
/* -------------------------------------------------------------------------- */
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{
    uint32_t temp1;
    uint32_t temp2;

    temp2 = pI2CHandle->pI2Cx->CR2 & I2C_CR2_ITERREN;

    /* Bus error */
    temp1 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_BERR;
    if ((temp1 != 0U) && (temp2 != 0U))
    {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_BERR;
        I2C_AbortTransfer(pI2CHandle);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_ERROR_BERR);
    }

    /* Arbitration lost */
    temp1 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_ARLO;
    if ((temp1 != 0U) && (temp2 != 0U))
    {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_ARLO;
        I2C_AbortTransfer(pI2CHandle);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_ERROR_ARLO);
    }

    /* ACK failure */
    temp1 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_AF;
    if ((temp1 != 0U) && (temp2 != 0U))
    {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_AF;
        I2C_AbortTransfer(pI2CHandle);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_ERROR_AF);
    }

    /* Overrun / underrun */
    temp1 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_OVR;
    if ((temp1 != 0U) && (temp2 != 0U))
    {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_OVR;
        I2C_AbortTransfer(pI2CHandle);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_ERROR_OVR);
    }

    /* Timeout */
    temp1 = pI2CHandle->pI2Cx->SR1 & I2C_FLAG_SR1_TIMEOUT;
    if ((temp1 != 0U) && (temp2 != 0U))
    {
        pI2CHandle->pI2Cx->SR1 &= ~I2C_FLAG_SR1_TIMEOUT;
        I2C_AbortTransfer(pI2CHandle);
        I2C_ApplicationEventCallback(pI2CHandle, I2C_APP_ERROR_TIMEOUT);
    }
}

/* -------------------------------------------------------------------------- */
/* Slave data helpers                                                         */
/* -------------------------------------------------------------------------- */
void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data)
{
    pI2C->DR = data;
}

uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C)
{
    return (uint8_t)pI2C->DR;
}

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2C, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pI2C->CR2 |= I2C_CR2_ITERREN;
        pI2C->CR2 |= I2C_CR2_ITEVTEN;
        pI2C->CR2 |= I2C_CR2_ITBUFEN;
    }
    else
    {
        pI2C->CR2 &= ~I2C_CR2_ITERREN;
        pI2C->CR2 &= ~I2C_CR2_ITEVTEN;
        pI2C->CR2 &= ~I2C_CR2_ITBUFEN;
    }
}

/* -------------------------------------------------------------------------- */
/* Flag status helpers                                                        */
/* -------------------------------------------------------------------------- */
uint8_t I2C_GetFlagStatus_SR1(I2C_Handle_t *pI2CHandle, uint32_t Flag)
{
    return ((pI2CHandle->pI2Cx->SR1 & Flag) != 0U) ? FLAG_SET : FLAG_RESET;
}

uint8_t I2C_GetFlagStatus_SR2(I2C_Handle_t *pI2CHandle, uint32_t Flag)
{
    return ((pI2CHandle->pI2Cx->SR2 & Flag) != 0U) ? FLAG_SET : FLAG_RESET;
}

/* -------------------------------------------------------------------------- */
/* Weak callback                                                              */
/* -------------------------------------------------------------------------- */
__weak void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEvent)
{
    (void)pI2CHandle;
    (void)AppEvent;
}
