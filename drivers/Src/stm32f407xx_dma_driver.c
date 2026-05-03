/*
 * stm32f407xx_dma_driver.c
 *
 *  Created on: Apr 23, 2026
 *      Author: Rasoul
 */

#include "stm32f407xx_dma_driver.h"


void DMA_PeriClockControl(DMA_RegDef_t *pDMAx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		if (pDMAx == DMA1)
		{
			DMA1_PCLK_EN();
		}else if(pDMAx == DMA2)
		{
			DMA2_PCLK_EN();
		}
	}else
	{
		if (pDMAx == DMA1)
		{
			DMA1_PCLK_DI();
		}else if(pDMAx == DMA2)
		{
			DMA2_PCLK_DI();
		}
	}

}
void DMA_Init(DMA_Handle_t *pDMAHandle)
{
    uint32_t temp = 0;

    // 1. Enable peripheral clock
    DMA_PeriClockControl(pDMAHandle->pDMAx, ENABLE);

    // 2. Disable stream before configuration
    pDMAHandle->pStream->CR &= ~DMA_SxCR_EN;

    // 3. Wait until stream is disabled
    while (pDMAHandle->pStream->CR & DMA_SxCR_EN);

    // 4. Configure channel selection
    temp |= (pDMAHandle->DMA_Config.Channel << DMA_SxCR_CHSEL_Pos);

    // 5. Configure direction
    temp |= (pDMAHandle->DMA_Config.Direction << DMA_SxCR_DIR_Pos);

    // 6. Configure peripheral increment
    if (pDMAHandle->DMA_Config.PeriphInc == DMA_PINC_ENABLE)
     {
         temp |= DMA_SxCR_PINC;
     }else
     {
         temp &= ~DMA_SxCR_PINC;

     }

    // 7. Configure memory increment
    if (pDMAHandle->DMA_Config.MemInc == DMA_MINC_ENABLE)
     {
         temp |= DMA_SxCR_MINC;
     }else
     {
         temp &= ~DMA_SxCR_MINC;

     }

    // 8. Configure peripheral data size
    temp |= (pDMAHandle->DMA_Config.PeriphDataSize << DMA_SxCR_PSIZE_Pos);

    // 9. Configure memory data size
    temp |= (pDMAHandle->DMA_Config.MemDataSize << DMA_SxCR_MSIZE_Pos);

    // 10. Configure priority
    temp |= (pDMAHandle->DMA_Config.Priority << DMA_SxCR_PL_Pos);

    // 11. Configure normal/circular mode

    if (pDMAHandle->DMA_Config.Mode == DMA_MODE_CIRCULAR)
    {
    	temp |= DMA_SxCR_CIRC;
    }else
    {
    	temp &= ~(DMA_SxCR_CIRC);
    }

    // 12. Enable transfer complete interrupt
    temp |= DMA_SxCR_TCIE;

    // 13. Write CR
    pDMAHandle->pStream->CR = temp;

    // 14. Configure FIFO register for direct mode first
    pDMAHandle->pStream->FCR &= ~DMA_SxFCR_DMDIS;
}

void DMA_DeInitStream(DMA_Handle_t *pDMAHandle)
{
    /*
     * 1. Disable the stream
     */
	DMA_DisableStream(pDMAHandle);

    /*
     * 2. Clear all pending flags for this stream
     */
    DMA_ClearFlags(pDMAHandle);

    /*
     * 3. Reset stream registers
     */
    pDMAHandle->pStream->CR  = 0U;   // CR
    pDMAHandle->pStream->NDTR  = 0U;   // NDTR
    pDMAHandle->pStream->PAR  = 0U;   // PAR
    pDMAHandle->pStream->M0AR  = 0U;   // M0AR
    pDMAHandle->pStream->M1AR  = 0U;   // M1AR

    /*
     * 4. Reset FIFO control register
     *    Check reset value from reference manual.
     *    Do not assume it is always 0.
     */
    pDMAHandle->pStream->FCR  = DMA_SxFCR_RESET_VALUE;
}

void DMA_ConfigAddresses(DMA_Handle_t *pDMAHandle,
                         uint32_t PeriphAddr,
                         uint32_t MemAddr)
{
    /*
     * Peripheral address register
     */
    pDMAHandle->pStream->PAR = PeriphAddr;

    /*
     * Memory 0 address register
     */
    pDMAHandle->pStream->M0AR = MemAddr;
}

void DMA_SetDataLength(DMA_Handle_t *pDMAHandle, uint32_t Len)
{
    /*
     * Number of data items to transfer.
     * Remember: this is NOT always bytes.
     * It depends on MSIZE/PSIZE.
     */
    pDMAHandle->pStream->NDTR = Len;
}


void DMA_EnableStream(DMA_Handle_t *pDMAHandle)
{
    /*
     * Enable stream by setting EN bit in SxCR.
     */
    pDMAHandle->pStream->CR |= DMA_SxCR_EN;
}

void DMA_DisableStream(DMA_Handle_t *pDMAHandle)
{
    /*
     * Clear EN bit first.
     */
    pDMAHandle->pStream->CR &= ~DMA_SxCR_EN;

    /*
     * Wait until hardware confirms stream is disabled.
     */
    while (pDMAHandle->pStream->CR & DMA_SxCR_EN);
}

uint8_t DMA_GetFlagStatus(DMA_Handle_t *pDMAHandle, uint32_t FlagName)
{
    uint32_t status = 0U;

    /*
     * Streams 0-3 use LISR.
     * Streams 4-7 use HISR.
     */
    if (pDMAHandle->StreamNumber <= 3U)
    {
        status = pDMAHandle->pDMAx->LISR & FlagName;
    }
    else
    {
        status = pDMAHandle->pDMAx->HISR & FlagName;
    }

    if (status != 0U)
    {
        return FLAG_SET;
    }

    return FLAG_RESET;
}


void DMA_ClearFlags(DMA_Handle_t *pDMAHandle)
{
    uint32_t flag_mask = 0U;

    /*
     * Streams 0,1,2,3 use LIFCR.
     * Streams 4,5,6,7 use HIFCR.
     */

    if (pDMAHandle->StreamNumber <= 3U)
    {
        if (pDMAHandle->StreamNumber == 0U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_0_4;
        }
        else if (pDMAHandle->StreamNumber == 1U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_1_5;
        }
        else if (pDMAHandle->StreamNumber == 2U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_2_6;
        }
        else if (pDMAHandle->StreamNumber == 3U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_3_7;
        }

        pDMAHandle->pDMAx->LIFCR = flag_mask;
    }
    else
    {
        if (pDMAHandle->StreamNumber == 4U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_0_4;
        }
        else if (pDMAHandle->StreamNumber == 5U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_1_5;
        }
        else if (pDMAHandle->StreamNumber == 6U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_2_6;
        }
        else if (pDMAHandle->StreamNumber == 7U)
        {
            flag_mask = DMA_CLEAR_ALL_FLAGS_3_7;
        }

        pDMAHandle->pDMAx->HIFCR = flag_mask;
    }
}


void DMA_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
    uint8_t reg = IRQNumber / 32U;
    uint8_t bit = IRQNumber % 32U;

    if (EnOrDi == ENABLE)
    {
        if (reg == 0U)
        {
            *NVIC_ISER0 |= (1U << bit);
        }
        else if (reg == 1U)
        {
            *NVIC_ISER1 |= (1U << bit);
        }
        else if (reg == 2U)
        {
            *NVIC_ISER2 |= (1U << bit);
        }
        else if (reg == 3U)
        {
            *NVIC_ISER3 |= (1U << bit);
        }
    }
    else
    {
        if (reg == 0U)
        {
            *NVIC_ICER0 |= (1U << bit);
        }
        else if (reg == 1U)
        {
            *NVIC_ICER1 |= (1U << bit);
        }
        else if (reg == 2U)
        {
            *NVIC_ICER2 |= (1U << bit);
        }
        else if (reg == 3U)
        {
            *NVIC_ICER3 |= (1U << bit);
        }
    }
}

void DMA_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    uint8_t iprx    = IRQNumber / 4U;
    uint8_t section = IRQNumber % 4U;
    uint8_t shift   = (section * 8U) + (8U - NO_PR_BITS_IMPLEMENTED);

    /* Clear old priority bits */
    NVIC_IPR_BASE_ADDR[iprx] &= ~(0xFFU << (section * 8U));

    /* Write new priority */
    NVIC_IPR_BASE_ADDR[iprx] |= ((IRQPriority & 0x0FU) << shift);
}

