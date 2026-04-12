/*
 * stm32f407xx_tim_driver.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Rasoul
 */

#include "stm32f407xx_tim_driver.h"

/* ========================= Peripheral clock control ========================= */

void TIM_PeriClockControl(TIM_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		if (pTIMx == TIM2)
		{
			TIM2_PCLK_EN();
		}else if(pTIMx == TIM3)
		{
			TIM3_PCLK_EN();
		}else if (pTIMx == TIM4)
		{
			TIM4_PCLK_EN();
		}else if (pTIMx == TIM5)
		{
			TIM5_PCLK_EN();
		}
	}else
	{
		if (pTIMx == TIM2)
		{
			TIM2_PCLK_DI();
		}else if(pTIMx == TIM3)
		{
			TIM3_PCLK_DI();
		}else if (pTIMx == TIM4)
		{
			TIM4_PCLK_DI();
		}else if (pTIMx == TIM5)
		{
			TIM5_PCLK_DI();
		}
	}
}

/* =========================  Init / DE-init ========================= */

void TIM_Init(TIM_Handle_t *pTIMHandle)
{
	// ENABLE Peripheral CLOCK
	TIM_PeriClockControl(pTIMHandle->pTIMx, ENABLE);


	// DISABLE Counter before configuration
	TIM_PeripheralControl(pTIMHandle->pTIMx, DISABLE);

	// WRITE PSC
	pTIMHandle->pTIMx->PSC = pTIMHandle->TIM_Config.Prescaler;

	// WRITE ARR
	pTIMHandle->pTIMx->ARR = pTIMHandle->TIM_Config.AutoReload;

	// Generate an Update event so PSC/ARR are loaded immediately
	pTIMHandle->pTIMx->EGR |= TIM_EGR_UG;

	// Optionally clear status flag
	pTIMHandle->pTIMx->SR &= ~TIM_SR_UIF;

}


void TIM_DeInit(TIM_RegDef_t *pTIMx)
{
	if (pTIMx == TIM2)
	{
		TIM2_REG_RESET();
	}else if (pTIMx == TIM3)
	{
		TIM3_REG_RESET();
	}else if (pTIMx == TIM4)
	{
		TIM4_REG_RESET();
	}else if (pTIMx == TIM5)
	{
		TIM5_REG_RESET();
	}
}

/* ========================= Peripheral control ========================= */

void TIM_PeripheralControl(TIM_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pTIMx->CR1 |= TIM_CR1_CEN;
	}else
	{
		pTIMx->CR1 &= ~TIM_CR1_CEN;
	}
}

/* ========================= IRQ configuration ========================= */

void TIM_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
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

void TIM_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    uint8_t iprx    = IRQNumber / 4U;
    uint8_t section = IRQNumber % 4U;
    uint8_t shift   = (section * 8U) + (8U - NO_PR_BITS_IMPLEMENTED);

    /* Clear old priority bits */
    NVIC_IPR_BASE_ADDR[iprx] &= ~(0xFFU << (section * 8U));

    /* Write new priority */
    NVIC_IPR_BASE_ADDR[iprx] |= ((IRQPriority & 0x0FU) << shift);
}


void TIM_EnableUpdateInterrupt(TIM_RegDef_t *pTIMx, uint8_t IRQNumber)
{
	// Enable Update Interrupt in timer
	pTIMx->DIER |= TIM_DIER_UIE;

	// Enable interrupt in NVIC
	TIM_IRQInterruptConfig(IRQNumber, ENABLE);
}
