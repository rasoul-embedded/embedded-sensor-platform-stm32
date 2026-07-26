/*
 * stm32f407xx_clock.c
 *
 *  Created on: Jun 28, 2026
 *      Author: rasoul
 */

#include "stm32f407xx_clock.h"
#include "stm32f407xx_gpio_driver.h"

#define CLOCK_TIMEOUT_VALUE 10000U


static uint32_t Clock_GetAHBDivider(void)
{
    uint32_t hpre;

    hpre = RCC->CFGR & RCC_CFGR_HPRE_MASK;

    switch (hpre)
    {
        case RCC_CFGR_HPRE_DIV1:
            return 1U;

        case RCC_CFGR_HPRE_DIV2:
            return 2U;

        case RCC_CFGR_HPRE_DIV4:
            return 4U;

        case RCC_CFGR_HPRE_DIV8:
            return 8U;

        case RCC_CFGR_HPRE_DIV16:
            return 16U;

        case RCC_CFGR_HPRE_DIV64:
            return 64U;

        case RCC_CFGR_HPRE_DIV128:
            return 128U;

        case RCC_CFGR_HPRE_DIV256:
            return 256U;

        case RCC_CFGR_HPRE_DIV512:
            return 512U;

        default:
            return 1U;
    }
}


static uint32_t Clock_GetAPB1Divider(void)
{
    uint32_t ppre1;

    ppre1 = RCC->CFGR & RCC_CFGR_PPRE1_MASK;

    switch (ppre1)
    {
        case RCC_CFGR_PPRE1_DIV1:
            return 1U;

        case RCC_CFGR_PPRE1_DIV2:
            return 2U;

        case RCC_CFGR_PPRE1_DIV4:
            return 4U;

        case RCC_CFGR_PPRE1_DIV8:
            return 8U;

        case RCC_CFGR_PPRE1_DIV16:
            return 16U;

        default:
            return 1U;
    }
}

static uint32_t Clock_GetAPB2Divider(void)
{
    uint32_t ppre2;

    ppre2 = RCC->CFGR & RCC_CFGR_PPRE2_MASK;

    switch (ppre2)
    {
        case RCC_CFGR_PPRE2_DIV1:
            return 1U;

        case RCC_CFGR_PPRE2_DIV2:
            return 2U;

        case RCC_CFGR_PPRE2_DIV4:
            return 4U;

        case RCC_CFGR_PPRE2_DIV8:
            return 8U;

        case RCC_CFGR_PPRE2_DIV16:
            return 16U;

        default:
            return 1U;
    }
}



Clock_Status_t Clock_EnableHSE_Bypass(void)
{
    uint32_t timeout = CLOCK_TIMEOUT_VALUE;


    /* 1. Enable HSE bypass mode */
    RCC->CR |= RCC_CR_HSEBYP;

    /* 2. Enable HSE */
    RCC->CR |= RCC_CR_HSEON;

    /* 3. Wait until HSE is ready */
    while (!(RCC->CR & RCC_CR_HSERDY))
    {
        if (timeout == 0U)
        {
            return CLOCK_TIMEOUT;
        }

        timeout--;
    }

    return CLOCK_OK;
}


Clock_Status_t Clock_EnableHSE_Crystal(void)
{
    uint32_t timeout = CLOCK_TIMEOUT_VALUE;

    /* 1. Disable HSE bypass mode */
    RCC->CR &= ~RCC_CR_HSEBYP;

    /* 2. Enable HSE */
    RCC->CR |= RCC_CR_HSEON;

    /* 3. Wait until HSE is ready */
    while (!(RCC->CR & RCC_CR_HSERDY))
    {
        if (timeout == 0U)
        {
            return CLOCK_TIMEOUT;
        }

        timeout--;
    }

    return CLOCK_OK;
}


Clock_Status_t Clock_SetSystemClock_HSE_8MHz(void)
{
    uint32_t timeout = CLOCK_TIMEOUT_VALUE;

    /*
     * Goal:
     * HSE    = 8 MHz
     * SYSCLK = 8 MHz
     * HCLK   = 4 MHz
     * PCLK1  = 2 MHz
     * PCLK2  = 2 MHz
     */

    /*
     * 1. Configure Flash latency = 0 wait states.
     */

    FLASH->ACR &= ~FLASH_ACR_LATENCY_MASK;
    FLASH->ACR |= FLASH_ACR_LATENCY_0WS;

    /*
     * 2. Configure AHB prescaler = /2.
     */

    RCC->CFGR &= ~RCC_CFGR_HPRE_MASK;
    RCC->CFGR |=  RCC_CFGR_HPRE_DIV2;


    /*
     * 3. Configure APB1 prescaler = /2.
     */
    RCC->CFGR &= ~RCC_CFGR_PPRE1_MASK;
    RCC->CFGR |=  RCC_CFGR_PPRE1_DIV2;

    /*
     * 4. Configure APB2 prescaler = /2.
     */

    RCC->CFGR &= ~RCC_CFGR_PPRE2_MASK;
    RCC->CFGR |=  RCC_CFGR_PPRE2_DIV2;

    /*
     * 5. Select HSE as SYSCLK source.
     */

    RCC->CFGR &= ~RCC_CFGR_SW_MASK;
    RCC->CFGR |=  RCC_CFGR_SW_HSE;

    /*
     * 6. Wait until SWS confirms HSE is SYSCLK.
     */
    while((RCC->CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_HSE)
    {
        if (timeout == 0U)
        {
            return CLOCK_TIMEOUT;
        }

        timeout--;
    }

    return CLOCK_OK;
}


uint32_t Clock_GetSysClockFreq(void)
{
    uint32_t sysclk_source;
    uint32_t pll_source_hz;
    uint32_t pllm;
    uint32_t plln;
    uint32_t pllp;
    uint32_t pllsrc;

    sysclk_source = RCC->CFGR & RCC_CFGR_SWS_MASK;

    if (sysclk_source == RCC_CFGR_SWS_HSI)
    {
        return MCU_HSI_VALUE_HZ;
    }
    else if (sysclk_source == RCC_CFGR_SWS_HSE)
    {
        return MCU_HSE_VALUE_HZ;
    }
    else if (sysclk_source == RCC_CFGR_SWS_PLL)
    {
        /*
         * Read PLL source.
         */
        pllsrc = RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC_MASK;

        if (pllsrc == RCC_PLLCFGR_PLLSRC_HSI)
        {
            pll_source_hz = MCU_HSI_VALUE_HZ;
        }
        else if (pllsrc == RCC_PLLCFGR_PLLSRC_HSE)
        {
            pll_source_hz = MCU_HSE_VALUE_HZ;
        }
        else
        {
            return 0U;
        }

        /*
         * Extract PLLM, PLLN, PLLP.
         */
        pllm = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM_MASK) >> RCC_PLLCFGR_PLLM_POS;
        plln = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN_MASK) >> RCC_PLLCFGR_PLLN_POS;

        /*
         * PLLP is encoded:
         * 00 = /2
         * 01 = /4
         * 10 = /6
         * 11 = /8
         */
        pllp = (RCC->PLLCFGR & RCC_PLLCFGR_PLLP_MASK) >> RCC_PLLCFGR_PLLP_POS;
        pllp = (pllp + 1U) * 2U;

        /*
         * Avoid division by zero.
         */
        if (pllm == 0U)
        {
            return 0U;
        }

        /*
         * PLLCLK = (PLL source / PLLM) * PLLN / PLLP
         */
        return (pll_source_hz / pllm) * plln / pllp;
    }
    else
    {
        return 0U;
    }
}

uint32_t Clock_GetHCLKFreq(void)
{
    uint32_t sysclk;
    uint32_t ahb_divider;

    sysclk = Clock_GetSysClockFreq();
    ahb_divider = Clock_GetAHBDivider();

    return sysclk / ahb_divider;
}

uint32_t Clock_GetPCLK1Freq(void)
{
    uint32_t hclk;
    uint32_t apb1_divider;

    hclk = Clock_GetHCLKFreq();
    apb1_divider = Clock_GetAPB1Divider();

    return hclk / apb1_divider;
}


uint32_t Clock_GetPCLK2Freq(void)
{
    uint32_t hclk;
    uint32_t apb2_divider;

    hclk = Clock_GetHCLKFreq();
    apb2_divider = Clock_GetAPB2Divider();

    return hclk / apb2_divider;
}


void Clock_OutputSYSCLK_MCO2_PC9(void)
{

	GPIO_Handle_t gpioc_handle;
	gpioc_handle.pGPIOx = GPIOC;
	gpioc_handle.GPIO_PinConfig.GPIO_PinAltFunMode = 0;
	gpioc_handle.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	gpioc_handle.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
	gpioc_handle.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
	gpioc_handle.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	gpioc_handle.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&gpioc_handle);

	RCC->CFGR &= ~RCC_CFGR_MCO2_MASK;
	RCC->CFGR |=  RCC_CFGR_MCO2_SYSCLK;

	RCC->CFGR &= ~RCC_CFGR_MCO2PRE_MASK;
	RCC->CFGR |=  RCC_CFGR_MCO2PRE_DIV4;

}

Clock_Status_t Clock_SetSystemClock_PLL_HSE_84MHz(void)
{
	uint32_t timeout = CLOCK_TIMEOUT_VALUE;

	// 1. Enable HSE
	Clock_EnableHSE_Bypass();

	if (!(RCC->CR & RCC_CR_HSERDY))
	{
		return CLOCK_ERROR;
	}
	// 2. Disable PLL if it is ON
	RCC->CR &= ~(RCC_CR_PLLON);

	// 3. Wait Until PLLRDY becomes 0

	while (RCC->CR & RCC_CR_PLLRDY)
	{
		if (timeout == 0U)
		{
			return CLOCK_TIMEOUT;
		}

		timeout--;
	}

	// 4. Configure PLLCFGR
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC_MASK);
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_MASK);
	RCC->PLLCFGR |= (8U << RCC_PLLCFGR_PLLM_POS);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN_MASK);
	RCC->PLLCFGR |= (168U << RCC_PLLCFGR_PLLN_POS);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLP_MASK);
	RCC->PLLCFGR |= ( RCC_PLLCFGR_PLLP_DIV2);
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLQ_MASK);
	RCC->PLLCFGR |= (7U << RCC_PLLCFGR_PLLQ_POS);

	// 5. Configure Flash latency


    FLASH->ACR &= ~FLASH_ACR_LATENCY_MASK;
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;


	// 6. Enable PLL
	RCC->CR |= (RCC_CR_PLLON);

	// 7. Wait for PLLRDY
	timeout = CLOCK_TIMEOUT_VALUE;
	while (!(RCC->CR & RCC_CR_PLLRDY))
	{
		if (timeout == 0U)
		{
			return CLOCK_TIMEOUT;
		}

		timeout--;
	}


	// 8. Configure AHB/APB prescalers


    RCC->CFGR &= ~RCC_CFGR_HPRE_MASK;
    RCC->CFGR |=  RCC_CFGR_HPRE_DIV1;

    RCC->CFGR &= ~RCC_CFGR_PPRE1_MASK;
    RCC->CFGR |=  RCC_CFGR_PPRE1_DIV2;


    RCC->CFGR &= ~RCC_CFGR_PPRE2_MASK;
    RCC->CFGR |=  RCC_CFGR_PPRE2_DIV1;

	// 9. Switch SYSCLK to PLL
    RCC->CFGR &= ~(RCC_CFGR_SW_MASK);
    RCC->CFGR |= (RCC_CFGR_SW_PLL);

	// 10. Wait until SWS = PLL

	timeout = CLOCK_TIMEOUT_VALUE;

    while ((RCC->CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL)
	{
		if (timeout == 0U)
		{
			return CLOCK_TIMEOUT;
		}
		timeout--;
	}


    return CLOCK_OK;
}
