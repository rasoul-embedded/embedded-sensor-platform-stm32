/*
 * stm32f407xx_usart_driver.c
 *
 *  Created on: Jan 28, 2026
 *      Author: rasoul
 *
 *  NOTE:
 *  - This driver implements basic USART init, blocking Tx/Rx, and interrupt-based Tx/Rx.
 *  - Baud-rate generation supports OVER8 = 0 or 1 (oversampling by 16 or 8) using RCC_GetPCLKxValue().
 */

#include "stm32f407xx_usart_driver.h"
#include "stm32f407xx_rcc_driver.h"   /* for RCC_GetPCLK1Value / RCC_GetPCLK2Value */
#include <stddef.h>                  /* NULL */

/* ========================= NVIC functions ========================= */

void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
    uint8_t reg = IRQNumber / 32U;
    uint8_t bit = IRQNumber % 32U;

    if (EnorDi == ENABLE)
    {
    	if      (reg == 0U) *NVIC_ISER0 |= (1U << bit);
    	else if (reg == 1U) *NVIC_ISER1 |= (1U << bit);
    	else if (reg == 2U) *NVIC_ISER2 |= (1U << bit);
    	else if (reg == 3U) *NVIC_ISER3 |= (1U << bit);
    }
    else
    {
        if      (reg == 0U) *NVIC_ICER0 |= (1U << bit);
        else if (reg == 1U) *NVIC_ICER1 |= (1U << bit);
        else if (reg == 2U) *NVIC_ICER2 |= (1U << bit);
        else if (reg == 3U) *NVIC_ICER3 |= (1U << bit);
    }
}

void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    uint8_t iprx    = IRQNumber / 4U;
    uint8_t section = IRQNumber % 4U;

    uint8_t shift = (section * 8U) + (8U - NO_PR_BITS_IMPLEMENTED);

    /* Clear entire 8-bit field first */
    NVIC_IPR_BASE_ADDR[iprx] &= ~(0xFFU << (section * 8U));

    /* Write new priority into the implemented bits */
    NVIC_IPR_BASE_ADDR[iprx] |= ((IRQPriority & 0xFU) << shift);
}

/* ========================= Driver functions ========================= */

void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        if (pUSARTx == USART1)      USART1_PCLK_EN();
        else if (pUSARTx == USART2) USART2_PCLK_EN();
        else if (pUSARTx == USART3) USART3_PCLK_EN();
        else if (pUSARTx == UART4)  UART4_PCLK_EN();
        else if (pUSARTx == UART5)  UART5_PCLK_EN();
        else if (pUSARTx == USART6) USART6_PCLK_EN();
    }
    else
    {
        if (pUSARTx == USART1)      USART1_PCLK_DI();
        else if (pUSARTx == USART2) USART2_PCLK_DI();
        else if (pUSARTx == USART3) USART3_PCLK_DI();
        else if (pUSARTx == UART4)  UART4_PCLK_DI();
        else if (pUSARTx == UART5)  UART5_PCLK_DI();
        else if (pUSARTx == USART6) USART6_PCLK_DI();
    }
}

void USART_DeInit(USART_RegDef_t *pUSARTx)
{
    if (pUSARTx == USART1)      USART1_REG_RESET();
    else if (pUSARTx == USART2) USART2_REG_RESET();
    else if (pUSARTx == USART3) USART3_REG_RESET();
    else if (pUSARTx == UART4)  UART4_REG_RESET();
    else if (pUSARTx == UART5)  UART5_REG_RESET();
    else if (pUSARTx == USART6) USART6_REG_RESET();
}

void USART_Init(USART_Handle_t *pUSARTHandle)
{
    uint32_t tempreg = 0U;

    USART_PeriClockControl(pUSARTHandle->pUSART, ENABLE);

    /* Disable before config */
    pUSARTHandle->pUSART->CR1 &= ~USART_CR1_UE;

    /******** CR1 ********/
    tempreg = 0U;

    if (pUSARTHandle->USART_Config.Mode == USART_MODE_ONLY_TX)
        tempreg |= USART_CR1_TE;
    else if (pUSARTHandle->USART_Config.Mode == USART_MODE_ONLY_RX)
        tempreg |= USART_CR1_RE;
    else
        tempreg |= (USART_CR1_TE | USART_CR1_RE);

    if (pUSARTHandle->USART_Config.WordLength == USART_WORDLEN_9BITS)
        tempreg |= (1U << USART_CR1_M_Pos);

    if (pUSARTHandle->USART_Config.ParityControl != USART_PARITY_DISABLE)
    {
        tempreg |= USART_CR1_PCE;
        if (pUSARTHandle->USART_Config.ParityControl == USART_PARITY_EN_ODD)
            tempreg |= (1U << USART_CR1_PS_Pos);
    }

    pUSARTHandle->pUSART->CR1 = tempreg;

    /******** CR2 ********/
    tempreg = 0U;
    tempreg |= ((uint32_t)pUSARTHandle->USART_Config.NoOfStopBits << USART_CR2_STOP_Pos);
    pUSARTHandle->pUSART->CR2 = tempreg;

    /******** CR3 ********/
    tempreg = 0U;
    if (pUSARTHandle->USART_Config.HWFlowControl == USART_HW_FLOW_CTRL_CTS)
        tempreg |= USART_CR3_CTSE;
    else if (pUSARTHandle->USART_Config.HWFlowControl == USART_HW_FLOW_CTRL_RTS)
        tempreg |= USART_CR3_RTSE;
    else if (pUSARTHandle->USART_Config.HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS)
        tempreg |= (USART_CR3_CTSE | USART_CR3_RTSE);

    pUSARTHandle->pUSART->CR3 = tempreg;

    /******** BRR ********/
    USART_SetBaudRate(pUSARTHandle->pUSART, pUSARTHandle->USART_Config.Baud);

    /* Enable USART */
    pUSARTHandle->pUSART->CR1 |= USART_CR1_UE;

    /* Initialize runtime state */
    pUSARTHandle->TxBusyState = USART_READY;
    pUSARTHandle->RxBusyState = USART_READY;
    pUSARTHandle->pTxBuffer = NULL;
    pUSARTHandle->pRxBuffer = NULL;
    pUSARTHandle->TxLen = 0U;
    pUSARTHandle->RxLen = 0U;
}

void USART_PeripheralControl(USART_RegDef_t *pUSART, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE) pUSART->CR1 |= USART_CR1_UE;
    else                 pUSART->CR1 &= ~USART_CR1_UE;
}

uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSART, uint32_t StatusFlagName)
{
    return ((pUSART->SR & StatusFlagName) ? FLAG_SET : FLAG_RESET);
}

void USART_ClearFlag(USART_RegDef_t *pUSART, uint32_t StatusFlagName)
{
    volatile uint32_t dummy;

    switch (StatusFlagName)
    {
        case USART_FLAG_RXNE:
        case USART_FLAG_ORE:
        case USART_FLAG_FE:
        case USART_FLAG_NE:
        case USART_FLAG_PE:
        case USART_FLAG_IDLE:
            dummy = pUSART->SR; (void)dummy;
            dummy = pUSART->DR; (void)dummy;
            break;

        case USART_FLAG_TC:
            /* TC can be cleared by writing 0 to it */
            pUSART->SR &= ~USART_FLAG_TC;
            break;

        default:
            break;
    }
}

/* ========================= Blocking send/recv ========================= */

void USART_SendData(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint16_t *pdata;

    while (Len > 0U)
    {
        while (USART_GetFlagStatus(pHandle->pUSART, USART_FLAG_TXE) == FLAG_RESET);

        if (pHandle->USART_Config.WordLength == USART_WORDLEN_9BITS)
        {
            if (pHandle->USART_Config.ParityControl == USART_PARITY_DISABLE)
            {
                if (Len < 2U) break; /* prevent underflow */
                pdata = (uint16_t*)pTxBuffer;
                pHandle->pUSART->DR = (*pdata & 0x01FFU);
                pTxBuffer += 2;
                Len -= 2;
            }
            else
            {
                pHandle->pUSART->DR = (*pTxBuffer & 0xFFU);
                pTxBuffer += 1;
                Len -= 1;
            }
        }
        else
        {
            pHandle->pUSART->DR = (*pTxBuffer & 0xFFU);
            pTxBuffer += 1;
            Len -= 1;
        }
    }

    while (USART_GetFlagStatus(pHandle->pUSART, USART_FLAG_TC) == FLAG_RESET);
}

void USART_ReceiveData(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    while (Len > 0U)
    {
        while (USART_GetFlagStatus(pHandle->pUSART, USART_FLAG_RXNE) == FLAG_RESET);

        if (pHandle->USART_Config.WordLength == USART_WORDLEN_9BITS)
        {
            if (pHandle->USART_Config.ParityControl == USART_PARITY_DISABLE)
            {
                if (Len < 2U) break; /* prevent underflow */
                *(uint16_t*)pRxBuffer = (uint16_t)(pHandle->pUSART->DR & 0x01FFU);
                pRxBuffer += 2;
                Len -= 2;
            }
            else
            {
                *pRxBuffer = (uint8_t)(pHandle->pUSART->DR & 0xFFU);
                pRxBuffer += 1;
                Len -= 1;
            }
        }
        else
        {
            *pRxBuffer = (uint8_t)(pHandle->pUSART->DR & 0xFFU);
            pRxBuffer += 1;
            Len -= 1;
        }
    }
}

/* ========================= Interrupt send/recv ========================= */

uint8_t USART_SendDataIT(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t txstate = pHandle->TxBusyState;

    if (txstate != USART_BUSY_IN_TX)
    {
        pHandle->TxLen       = Len;
        pHandle->pTxBuffer   = pTxBuffer;
        pHandle->TxBusyState = USART_BUSY_IN_TX;

        /* Clear TC so we don't get an immediate TC interrupt from a previous transfer */
        USART_ClearFlag(pHandle->pUSART, USART_FLAG_TC);

        pHandle->pUSART->CR1 |= USART_CR1_TXEIE;
        pHandle->pUSART->CR1 &= ~USART_CR1_TCIE; /* enable at end */
    }

    return txstate;
}

uint8_t USART_ReceiveDataIT(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t rxstate = pHandle->RxBusyState;

    if (rxstate != USART_BUSY_IN_RX)
    {
        pHandle->RxLen       = Len;
        pHandle->pRxBuffer   = pRxBuffer;
        pHandle->RxBusyState = USART_BUSY_IN_RX;

        pHandle->pUSART->CR1 |= USART_CR1_RXNEIE;
    }

    return rxstate;
}

void USART_IRQHandling(USART_Handle_t *pHandle)
{
    uint32_t sr  = pHandle->pUSART->SR;
    uint32_t cr1 = pHandle->pUSART->CR1;

    /******** TXE ********/
    if ((sr & USART_FLAG_TXE) && (cr1 & USART_CR1_TXEIE))
    {
        if (pHandle->TxBusyState == USART_BUSY_IN_TX)
        {
            if (pHandle->TxLen > 0U)
            {
                if (pHandle->USART_Config.WordLength == USART_WORDLEN_9BITS)
                {
                    if (pHandle->USART_Config.ParityControl == USART_PARITY_DISABLE)
                    {
                        if (pHandle->TxLen >= 2U)
                        {
                            uint16_t *pdata = (uint16_t*)pHandle->pTxBuffer;
                            pHandle->pUSART->DR = (*pdata & 0x01FFU);
                            pHandle->pTxBuffer += 2;
                            pHandle->TxLen     -= 2;
                        }
                        else
                        {
                            /* invalid length for 9-bit no-parity, stop */
                            pHandle->TxLen = 0U;
                        }
                    }
                    else
                    {
                        pHandle->pUSART->DR = (*pHandle->pTxBuffer & 0xFFU);
                        pHandle->pTxBuffer += 1;
                        pHandle->TxLen     -= 1;
                    }
                }
                else
                {
                    pHandle->pUSART->DR = (*pHandle->pTxBuffer & 0xFFU);
                    pHandle->pTxBuffer += 1;
                    pHandle->TxLen     -= 1;
                }
            }

            if (pHandle->TxLen == 0U)
            {
                pHandle->pUSART->CR1 &= ~USART_CR1_TXEIE;
                pHandle->pUSART->CR1 |=  USART_CR1_TCIE;
            }
        }
    }

    /******** TC ********/
    if ((sr & USART_FLAG_TC) && (cr1 & USART_CR1_TCIE))
    {
        if (pHandle->TxBusyState == USART_BUSY_IN_TX)
        {
            /* Clear TC first */
            USART_ClearFlag(pHandle->pUSART, USART_FLAG_TC);

            pHandle->pUSART->CR1 &= ~USART_CR1_TCIE;

            pHandle->TxBusyState = USART_READY;
            pHandle->pTxBuffer   = NULL;
            pHandle->TxLen       = 0U;

            USART_ApplicationEventCallback(pHandle, USART_EVENT_TX_CMPLT);
        }
    }

    /******** RXNE ********/
    if ((sr & USART_FLAG_RXNE) && (cr1 & USART_CR1_RXNEIE))
    {
        if (pHandle->RxBusyState == USART_BUSY_IN_RX)
        {
            if (pHandle->RxLen > 0U)
            {
                if (pHandle->USART_Config.WordLength == USART_WORDLEN_9BITS)
                {
                    if (pHandle->USART_Config.ParityControl == USART_PARITY_DISABLE)
                    {
                        if (pHandle->RxLen >= 2U)
                        {
                            *(uint16_t*)pHandle->pRxBuffer = (uint16_t)(pHandle->pUSART->DR & 0x01FFU);
                            pHandle->pRxBuffer += 2;
                            pHandle->RxLen     -= 2;
                        }
                        else
                        {
                            /* invalid length for 9-bit no-parity, stop */
                            pHandle->RxLen = 0U;
                            (void)pHandle->pUSART->DR; /* still read DR once */
                        }
                    }
                    else
                    {
                        *pHandle->pRxBuffer = (uint8_t)(pHandle->pUSART->DR & 0xFFU);
                        pHandle->pRxBuffer += 1;
                        pHandle->RxLen     -= 1;
                    }
                }
                else
                {
                    *pHandle->pRxBuffer = (uint8_t)(pHandle->pUSART->DR & 0xFFU);
                    pHandle->pRxBuffer += 1;
                    pHandle->RxLen     -= 1;
                }
            }

            if (pHandle->RxLen == 0U)
            {
                pHandle->pUSART->CR1 &= ~USART_CR1_RXNEIE;

                pHandle->RxBusyState = USART_READY;
                pHandle->pRxBuffer   = NULL;

                USART_ApplicationEventCallback(pHandle, USART_EVENT_RX_CMPLT);
            }
        }
    }
}

/* ========================= Baud rate ========================= */

void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate)
{
    uint32_t PCLKx;
    uint32_t usartdiv;
    uint32_t M_part, F_part;
    uint32_t tempreg = 0U;

    if (BaudRate == 0U) return;

    /* Get APB clock */
    if ((pUSARTx == USART1) || (pUSARTx == USART6))
        PCLKx = RCC_GetPCLK2Value();
    else
        PCLKx = RCC_GetPCLK1Value();

    if (PCLKx == 0U) return;

    /* Check OVER8 */
    if (pUSARTx->CR1 & USART_CR1_OVER8)
    {
        /* oversampling by 8 */
        usartdiv = ((25U * PCLKx) / (2U * BaudRate));
    }
    else
    {
        /* oversampling by 16 */
        usartdiv = ((25U * PCLKx) / (4U * BaudRate));
    }

    /* Mantissa */
    M_part = usartdiv / 100U;
    tempreg |= (M_part << 4U);

    /* Fraction */
    F_part = (usartdiv - (M_part * 100U));

    if (pUSARTx->CR1 & USART_CR1_OVER8)
    {
        F_part = (((F_part * 8U) + 50U) / 100U) & 0x07U;
    }
    else
    {
        F_part = (((F_part * 16U) + 50U) / 100U) & 0x0FU;
    }

    tempreg |= F_part;

    pUSARTx->BRR = tempreg;
}

/* ========================= Weak callback default ========================= */

__weak void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, USART_AppEvent_t AppEvent)
{
    (void)pUSARTHandle;
    (void)AppEvent;
    /* User can override this in application code */
}
