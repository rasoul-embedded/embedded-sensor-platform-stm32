/* stm32f407xx_usart_driver.h
 *
 *  Created on: Jan 28, 2026
 *      Author: rasoul
 */

#ifndef INC_STM32F407XX_USART_DRIVER_H_
#define INC_STM32F407XX_USART_DRIVER_H_

#include "stm32f407xx.h"

/*
 *  Configuration structure for USARTx peripheral
 */
typedef struct
{
    uint8_t  Mode;           /* @USART_MODE_ */
    uint32_t Baud;           /* @USART_Baud_ */
    uint8_t  NoOfStopBits;   /* @USART_STOPBITS_ */
    uint8_t  WordLength;     /* @USART_WORDLEN_ */
    uint8_t  ParityControl;  /* @USART_PARITY_ */
    uint8_t  HWFlowControl;  /* @USART_HW_FLOW_CTRL_ */
} USART_Config_t;

/*
 * Handle structure for USARTx peripheral
 */
typedef struct
{
    USART_RegDef_t  *pUSART;
    USART_Config_t   USART_Config;

    uint8_t  *pTxBuffer;
    uint32_t TxLen;
    uint8_t  TxBusyState;

    uint8_t  *pRxBuffer;
    uint32_t RxLen;
    uint8_t  RxBusyState;

} USART_Handle_t;

typedef enum
{
    USART_EVENT_TX_CMPLT = 0,
    USART_EVENT_RX_CMPLT,
    USART_EVENT_IDLE,
    USART_EVENT_CTS,
    USART_EVENT_PE,
    USART_EVENT_FE,
    USART_EVENT_NE,
    USART_EVENT_ORE
} USART_AppEvent_t;

/* ===================== Config macros ===================== */

/* USART MODE macros */
#define USART_MODE_ONLY_TX      0U
#define USART_MODE_ONLY_RX      1U
#define USART_MODE_TXRX         2U


/*USART Baud macros */
#define USART_STD_BAUD_1200      1200
#define USART_STD_BAUD_2400      2400
#define USART_STD_BAUD_9600      9600
#define USART_STD_BAUD_19200     19200
#define USART_STD_BAUD_38400     38400
#define USART_STD_BAUD_57600     57600
#define USART_STD_BAUD_115200    115200
#define USART_STD_BAUD_230400    230400
#define USART_STD_BAUD_460800    460800


/* USART WORDLENGTH macros */
#define USART_WORDLEN_8BITS     0U
#define USART_WORDLEN_9BITS     1U

/* USART ParityControl macros */
#define USART_PARITY_DISABLE    0U
#define USART_PARITY_EN_EVEN    1U
#define USART_PARITY_EN_ODD     2U

/* USART NoOfStopBits macros */
#define USART_STOPBITS_1        0U
#define USART_STOPBITS_0_5      1U
#define USART_STOPBITS_2        2U
#define USART_STOPBITS_1_5      3U

/* USART HWFlowControl macros */
#define USART_HW_FLOW_CTRL_NONE     0U
#define USART_HW_FLOW_CTRL_CTS      1U
#define USART_HW_FLOW_CTRL_RTS      2U
#define USART_HW_FLOW_CTRL_CTS_RTS  3U

/* USART State macros */
#define USART_READY        0U
#define USART_BUSY_IN_TX   1U
#define USART_BUSY_IN_RX   2U

/************************************************************************************
 * APIs supported by this driver
 ************************************************************************************/
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi);
void USART_Init(USART_Handle_t *pUSARTHandle);
void USART_DeInit(USART_RegDef_t *pUSARTx);

void USART_SendData(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len);
void USART_ReceiveData(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len);

uint8_t USART_SendDataIT(USART_Handle_t *pHandle, uint8_t *pTxBuffer, uint32_t Len);
uint8_t USART_ReceiveDataIT(USART_Handle_t *pHandle, uint8_t *pRxBuffer, uint32_t Len);

void USART_IRQHandling(USART_Handle_t *pHandle);
void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);

void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, USART_AppEvent_t AppEvent);

void USART_PeripheralControl(USART_RegDef_t *pUSART, uint8_t EnOrDi);
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSART, uint32_t StatusFlagName);
void USART_ClearFlag(USART_RegDef_t *pUSART, uint32_t StatusFlagName);
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate);

/************************************************************************************
 * Register Bit Definitions
 ************************************************************************************/

/* ================= USART_SR ================= */
#define USART_SR_PE_Pos     0U
#define USART_SR_FE_Pos     1U
#define USART_SR_NE_Pos     2U
#define USART_SR_ORE_Pos    3U
#define USART_SR_IDLE_Pos   4U
#define USART_SR_RXNE_Pos   5U
#define USART_SR_TC_Pos     6U
#define USART_SR_TXE_Pos    7U
#define USART_SR_CTS_Pos    9U

#define USART_FLAG_PE      (1U << USART_SR_PE_Pos)
#define USART_FLAG_FE      (1U << USART_SR_FE_Pos)
#define USART_FLAG_NE      (1U << USART_SR_NE_Pos)
#define USART_FLAG_ORE     (1U << USART_SR_ORE_Pos)
#define USART_FLAG_IDLE    (1U << USART_SR_IDLE_Pos)
#define USART_FLAG_RXNE    (1U << USART_SR_RXNE_Pos)
#define USART_FLAG_TC      (1U << USART_SR_TC_Pos)
#define USART_FLAG_TXE     (1U << USART_SR_TXE_Pos)
#define USART_FLAG_CTS     (1U << USART_SR_CTS_Pos)

/* ================= USART_CR1 ================= */
#define USART_CR1_UE_Pos        13U
#define USART_CR1_UE            (1U << USART_CR1_UE_Pos)

#define USART_CR1_M_Pos         12U

#define USART_CR1_PCE_Pos       10U
#define USART_CR1_PCE           (1U << USART_CR1_PCE_Pos)

#define USART_CR1_PS_Pos         9U

#define USART_CR1_TXEIE_Pos      7U
#define USART_CR1_TXEIE          (1U << USART_CR1_TXEIE_Pos)

#define USART_CR1_TCIE_Pos       6U
#define USART_CR1_TCIE           (1U << USART_CR1_TCIE_Pos)

#define USART_CR1_RXNEIE_Pos     5U
#define USART_CR1_RXNEIE         (1U << USART_CR1_RXNEIE_Pos)

#define USART_CR1_TE_Pos         3U
#define USART_CR1_TE             (1U << USART_CR1_TE_Pos)

#define USART_CR1_RE_Pos         2U
#define USART_CR1_RE             (1U << USART_CR1_RE_Pos)

#define USART_CR1_OVER8_Pos      15U
#define USART_CR1_OVER8          (1U << USART_CR1_OVER8_Pos)

/* ================= USART_CR2 ================= */
#define USART_CR2_STOP_Pos      12U

/* ================= USART_CR3 ================= */
#define USART_CR3_RTSE_Pos      8U
#define USART_CR3_RTSE          (1U << USART_CR3_RTSE_Pos)

#define USART_CR3_CTSE_Pos      9U
#define USART_CR3_CTSE          (1U << USART_CR3_CTSE_Pos)

#define USART_CR3_CTSIE_Pos     10U
#define USART_CR3_CTSIE         (1U << USART_CR3_CTSIE_Pos)

#define USART_CR3_DMAT_Pos      7U
#define USART_CR3_DMAT      	(1U << USART_CR3_DMAT_Pos)

#define USART_CR3_DMAR_Pos      6U
#define USART_CR3_DMAR      	(1U << USART_CR3_DMAR_Pos)

#endif /* INC_STM32F407XX_USART_DRIVER_H_ */
