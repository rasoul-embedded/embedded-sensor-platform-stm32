/*
 * stm32f407xx_tim_driver.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Rasoul
 */

#include "stm32f407xx_tim_driver.h"



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


/***********************************************************************************
 * 					These APIs are for the first TIMER family (Basic family)	   *
 * 									TIM6 and TIM7								   *
 ***********************************************************************************/


/* ========================= Peripheral clock control ========================= */

void TIM_Basic_PeriClockControl(TIM_Basic_RegDef_t *pTIMx, uint8_t EnOrDi)
{


	if (EnOrDi == ENABLE)
	{
		if (pTIMx == TIM6)
		{
			TIM6_PCLK_EN();
		}else if (pTIMx == TIM7)
		{
			TIM7_PCLK_EN();
		}
	}else if(EnOrDi == DISABLE)
	{
		if (pTIMx == TIM6)
		{
			TIM6_PCLK_DI();
		}else if (pTIMx == TIM7)
		{
			TIM7_PCLK_DI();
		}
	}
}

/* ================= Initialization ================= */

void TIM_Basic_Init(TIM_Basic_Handle_t *pTIMHandle)
{

	// Enable Peripheral Clock
	TIM_Basic_PeriClockControl(pTIMHandle->pTIMx, ENABLE);

	// Stop Counter before configuration
	TIM_Basic_PeripheralControl(pTIMHandle->pTIMx, DISABLE);

	// Configure the time base
	pTIMHandle->pTIMx->PSC = (pTIMHandle->TIM_Config.Prescaler & 0xFFFF);
	pTIMHandle->pTIMx->ARR = (pTIMHandle->TIM_Config.AutoReload & 0xFFFF);

	// Start counting from zero
	pTIMHandle->pTIMx->CNT = 0U;

	// Generate an Update event so PSC/ARR are loaded immediately
	pTIMHandle->pTIMx->EGR = TIM_EGR_UG;

	// Clear status flag
	pTIMHandle->pTIMx->SR &= ~TIM_SR_UIF;

	TIM_Basic_EnableUpdateInterrupt(
	        pTIMHandle->pTIMx,
	        pTIMHandle->TIM_Config.UpdateInterruptEnable);

}


void TIM_Basic_DeInit(TIM_Basic_RegDef_t *pTIMx)
{
	if (pTIMx == TIM6)
	{
		TIM6_REG_RESET();

	}else if (pTIMx == TIM7)
	{
		TIM7_REG_RESET();
	}
}

/* ========================= Peripheral control ========================= */

void TIM_Basic_PeripheralControl(TIM_Basic_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pTIMx->CR1 |= (TIM_CR1_CEN);
	}else
	{
		pTIMx->CR1 &= ~(TIM_CR1_CEN);
	}
}


void TIM_Basic_EnableUpdateInterrupt(TIM_Basic_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		// Enable Update Interrupt
		pTIMx->DIER |= TIM_DIER_UIE;
	}else
	{
	// Disable Update Interrupt
		pTIMx->DIER &= ~TIM_DIER_UIE;
	}
}


void TIM_Basic_IRQHandling(TIM_Basic_Handle_t *pTIMHandle)
{
	if ((pTIMHandle->pTIMx->SR & TIM_SR_UIF) && (pTIMHandle->pTIMx->DIER & TIM_DIER_UIE))
	{
        // Clear update flag
        pTIMHandle->pTIMx->SR &= ~TIM_SR_UIF;

        // Notify application
        TIM_Basic_ApplicationEventCallback(pTIMHandle);
	}
}


__attribute__((weak))
void TIM_Basic_ApplicationEventCallback(
        TIM_Basic_Handle_t *pTIMHandle)
{
    /* Default implementation */
    (void)pTIMHandle;
}


/***********************************************************************************
 * 					These APIs are for the second TIMER family (General Purpose)	   *
 * 									TIM2 to TIM5								   *
 ***********************************************************************************/

/* ========================= Peripheral clock control ========================= */

void TIM_GeneralPurpose_PeriClockControl(TIM_GeneralPurpose_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		if (pTIMx == TIM2)
		{
			TIM2_PCLK_EN();
		}else if (pTIMx == TIM3)
		{
			TIM3_PCLK_EN();
		}else if (pTIMx == TIM4)
		{
			TIM4_PCLK_EN();
		}else if (pTIMx == TIM5)
		{
			TIM5_PCLK_EN();
		}
	}else if(EnOrDi == DISABLE)
	{
		if (pTIMx == TIM2)
		{
			TIM2_PCLK_DI();
		}else if (pTIMx == TIM3)
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



/* ================= Initialization ================= */

void TIM_GeneralPurpose_Init(TIM_GeneralPurpose_Handle_t *pTIMHandle)
{
	// Enable Peripheral Clock
	TIM_GeneralPurpose_PeriClockControl(pTIMHandle->pTIMx, ENABLE);

	// Stop Counter before configuration
	TIM_GeneralPurpose_PeripheralControl(pTIMHandle->pTIMx, DISABLE);

	// Configure the time base
	pTIMHandle->pTIMx->PSC = (pTIMHandle->TIM_Config.Prescaler & 0xFFFF);

	if (pTIMHandle->pTIMx == TIM2 || pTIMHandle->pTIMx == TIM5)
	{
	    /* TIM2 and TIM5: 32-bit ARR */
		pTIMHandle->pTIMx->ARR = (pTIMHandle->TIM_Config.AutoReload);

	}else
	{
	    /* TIM3 and TIM4: 16-bit ARR */
		pTIMHandle->pTIMx->ARR = (pTIMHandle->TIM_Config.AutoReload & 0xFFFF);
	}

	// Start counting from zero
	pTIMHandle->pTIMx->CNT = 0U;

	// Generate an Update event so PSC/ARR are loaded immediately
	pTIMHandle->pTIMx->EGR = TIM_EGR_UG;


	/* Clear UIF generated by UG */
	pTIMHandle->pTIMx->SR = ~TIM_SR_UIF;

	// Clear status flag
	pTIMHandle->pTIMx->SR &= ~TIM_SR_UIF;

	TIM_GeneralPurpose_EnableUpdateInterrupt(
			pTIMHandle->pTIMx,
			pTIMHandle->TIM_Config.UpdateInterruptEnable);
}

void TIM_GeneralPurpose_DeInit(TIM_GeneralPurpose_RegDef_t *pTIMx)
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


void TIM_GeneralPurpose_PeripheralControl(TIM_GeneralPurpose_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pTIMx->CR1 |= (TIM_CR1_CEN);
	}else
	{
		pTIMx->CR1 &= ~(TIM_CR1_CEN);
	}
}


void TIM_GeneralPurpose_EnableUpdateInterrupt(TIM_GeneralPurpose_RegDef_t *pTIMx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		// Enable Update Interrupt
		pTIMx->DIER |= TIM_DIER_UIE;
	}else
	{
	// Disable Update Interrupt
		pTIMx->DIER &= ~TIM_DIER_UIE;
	}
}



void TIM_GeneralPurpose_IRQHandling(TIM_GeneralPurpose_Handle_t *pTIMHandle)
{
	if ((pTIMHandle->pTIMx->SR & TIM_SR_UIF) && (pTIMHandle->pTIMx->DIER & TIM_DIER_UIE))
	{
        // Clear update flag
        pTIMHandle->pTIMx->SR &= ~TIM_SR_UIF;

        // Notify application
        TIM_GeneralPurpose_ApplicationEventCallback(pTIMHandle);
	}

}

__attribute__((weak))
void TIM_GeneralPurpose_ApplicationEventCallback(
		TIM_GeneralPurpose_Handle_t *pTIMHandle)
{
    /* Default implementation */
    (void)pTIMHandle;
}

