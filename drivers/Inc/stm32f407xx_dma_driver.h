/*
 * stm32f407xx_dma_driver.h
 */

#ifndef INC_STM32F407XX_DMA_DRIVER_H_
#define INC_STM32F407XX_DMA_DRIVER_H_

#include "stm32f407xx.h"

#define DMA_SxFCR_RESET_VALUE   0x00000021U

/*
 * DMA configuration structure
 */
typedef struct
{
    uint8_t Channel;
    uint8_t Direction;
    uint8_t PeriphInc;
    uint8_t MemInc;
    uint8_t PeriphDataSize;
    uint8_t MemDataSize;
    uint8_t Mode;
    uint8_t Priority;
} DMA_Config_t;

/*
 * DMA handle structure
 */
typedef struct
{
    DMA_RegDef_t        *pDMAx;
    DMA_Stream_RegDef_t *pStream;
    uint8_t              StreamNumber;
    DMA_Config_t         DMA_Config;
} DMA_Handle_t;



/*
 * DMA Stream selection values
 */
#define DMA_STREAM_0              0U
#define DMA_STREAM_1              1U
#define DMA_STREAM_2              2U
#define DMA_STREAM_3              3U
#define DMA_STREAM_4              4U
#define DMA_STREAM_5              5U
#define DMA_STREAM_6              6U
#define DMA_STREAM_7              7U


/*
 * DMA channel selection values
 */
#define DMA_CHANNEL_0              0U
#define DMA_CHANNEL_1              1U
#define DMA_CHANNEL_2              2U
#define DMA_CHANNEL_3              3U
#define DMA_CHANNEL_4              4U
#define DMA_CHANNEL_5              5U
#define DMA_CHANNEL_6              6U
#define DMA_CHANNEL_7              7U




/*
 * DMA direction values
 */
#define DMA_DIR_P2M                0U
#define DMA_DIR_M2P                1U
#define DMA_DIR_M2M                2U


/*
 * Peripheral increment configuration
 */
#define DMA_PINC_DISABLE           0U
#define DMA_PINC_ENABLE            1U


/*
 * Memory increment configuration
 */
#define DMA_MINC_DISABLE           0U
#define DMA_MINC_ENABLE            1U


/*
 * Peripheral data size
 */
#define DMA_PSIZE_BYTE             0U
#define DMA_PSIZE_HALFWORD         1U
#define DMA_PSIZE_WORD             2U


/*
 * Memory data size
 */
#define DMA_MSIZE_BYTE             0U
#define DMA_MSIZE_HALFWORD         1U
#define DMA_MSIZE_WORD             2U


/*
 * DMA priority level
 */
#define DMA_PL_LOW                 0U
#define DMA_PL_MEDIUM              1U
#define DMA_PL_HIGH                2U
#define DMA_PL_VERYHIGH            3U

/*
 * DMA mode
 */
#define DMA_MODE_NORMAL            0U
#define DMA_MODE_CIRCULAR          1U


/*
 * Driver APIs
 */
void DMA_PeriClockControl(DMA_RegDef_t *pDMAx, uint8_t EnOrDi);

void DMA_Init(DMA_Handle_t *pDMAHandle);
void DMA_DeInitStream(DMA_Handle_t *pDMAHandle);

void DMA_ConfigAddresses(DMA_Handle_t *pDMAHandle,
                         uint32_t PeriphAddr,
                         uint32_t MemAddr);

void DMA_SetDataLength(DMA_Handle_t *pDMAHandle, uint32_t Len);

void DMA_EnableStream(DMA_Handle_t *pDMAHandle);
void DMA_DisableStream(DMA_Handle_t *pDMAHandle);

void DMA_ClearFlags(DMA_Handle_t *pDMAHandle);

uint8_t DMA_GetFlagStatus(DMA_Handle_t *pDMAHandle, uint32_t FlagName);

void DMA_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void DMA_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);


/*
 * DMA stream configuration register bit positions
 */
#define DMA_SxCR_CHSEL_Pos         25U
#define DMA_SxCR_DIR_Pos           6U
#define DMA_SxCR_PSIZE_Pos         11U
#define DMA_SxCR_MSIZE_Pos         13U
#define DMA_SxCR_PL_Pos            16U

/*
 * DMA stream configuration register bit masks
 */
#define DMA_SxCR_CHSEL_Msk    (0x7U << DMA_SxCR_CHSEL_Pos)
#define DMA_SxCR_DIR_Msk      (0x3U << DMA_SxCR_DIR_Pos)
#define DMA_SxCR_PSIZE_Msk    (0x3U << DMA_SxCR_PSIZE_Pos)
#define DMA_SxCR_MSIZE_Msk    (0x3U << DMA_SxCR_MSIZE_Pos)
#define DMA_SxCR_PL_Msk       (0x3U << DMA_SxCR_PL_Pos)


/*
 * DMA stream configuration register bit masks
 */
#define DMA_SxCR_EN                (1U << 0U)
#define DMA_SxCR_TCIE              (1U << 4U)
#define DMA_SxCR_PINC              (1U << 9U)
#define DMA_SxCR_MINC              (1U << 10U)
#define DMA_SxCR_CIRC              (1U << 8U)

/*
 * DMA stream FIFO control register bit masks
 */
#define DMA_SxFCR_DMDIS    		   (1U << 2U)



/*
 * DMA status flag masks
 * Use with LISR for streams 0-3
 * Use with HISR for streams 4-7
 */

/* Stream 0 / Stream 4 */
#define DMA_FLAG_FEIF0_4      (1U << 0U)
#define DMA_FLAG_DMEIF0_4     (1U << 2U)
#define DMA_FLAG_TEIF0_4      (1U << 3U)
#define DMA_FLAG_HTIF0_4      (1U << 4U)
#define DMA_FLAG_TCIF0_4      (1U << 5U)

/* Stream 1 / Stream 5 */
#define DMA_FLAG_FEIF1_5      (1U << 6U)
#define DMA_FLAG_DMEIF1_5     (1U << 8U)
#define DMA_FLAG_TEIF1_5      (1U << 9U)
#define DMA_FLAG_HTIF1_5      (1U << 10U)
#define DMA_FLAG_TCIF1_5      (1U << 11U)

/* Stream 2 / Stream 6 */
#define DMA_FLAG_FEIF2_6      (1U << 16U)
#define DMA_FLAG_DMEIF2_6     (1U << 18U)
#define DMA_FLAG_TEIF2_6      (1U << 19U)
#define DMA_FLAG_HTIF2_6      (1U << 20U)
#define DMA_FLAG_TCIF2_6      (1U << 21U)

/* Stream 3 / Stream 7 */
#define DMA_FLAG_FEIF3_7      (1U << 22U)
#define DMA_FLAG_DMEIF3_7     (1U << 24U)
#define DMA_FLAG_TEIF3_7      (1U << 25U)
#define DMA_FLAG_HTIF3_7      (1U << 26U)
#define DMA_FLAG_TCIF3_7      (1U << 27U)


/*
 * DMA clear-all flag masks
 * Write to LIFCR for streams 0-3
 * Write to HIFCR for streams 4-7
 */

#define DMA_CLEAR_ALL_FLAGS_0_4   (DMA_FLAG_FEIF0_4  | \
                                   DMA_FLAG_DMEIF0_4 | \
                                   DMA_FLAG_TEIF0_4  | \
                                   DMA_FLAG_HTIF0_4  | \
                                   DMA_FLAG_TCIF0_4)

#define DMA_CLEAR_ALL_FLAGS_1_5   (DMA_FLAG_FEIF1_5  | \
                                   DMA_FLAG_DMEIF1_5 | \
                                   DMA_FLAG_TEIF1_5  | \
                                   DMA_FLAG_HTIF1_5  | \
                                   DMA_FLAG_TCIF1_5)

#define DMA_CLEAR_ALL_FLAGS_2_6   (DMA_FLAG_FEIF2_6  | \
                                   DMA_FLAG_DMEIF2_6 | \
                                   DMA_FLAG_TEIF2_6  | \
                                   DMA_FLAG_HTIF2_6  | \
                                   DMA_FLAG_TCIF2_6)

#define DMA_CLEAR_ALL_FLAGS_3_7   (DMA_FLAG_FEIF3_7  | \
                                   DMA_FLAG_DMEIF3_7 | \
                                   DMA_FLAG_TEIF3_7  | \
                                   DMA_FLAG_HTIF3_7  | \
                                   DMA_FLAG_TCIF3_7)

#define DMA_TX_OK       0U
#define DMA_TX_BUSY     1U
#define DMA_TX_ERROR    2U

#endif /* INC_STM32F407XX_DMA_DRIVER_H_ */
