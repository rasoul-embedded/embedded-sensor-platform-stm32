/*
 * stm32f407xx_rcc_driver.c
 *
 *  Created on: Feb 3, 2026
 *      Author: rasoul
 */

#include "stm32f407xx_rcc_driver.h"

#define HSI_VALUE 16000000U
#define HSE_VALUE 8000000U

static const uint16_t AHB_Prescaler[8]  = {2,4,8,16,64,128,256,512};
static const uint8_t  APB_Prescaler[4]  = {2,4,8,16};


/**
 * @brief  Get PLL output clock (PLLCLK) in Hz.
 * @note   Uses PLLCFGR decoding: PLLM, PLLN, PLLP, PLLSRC.
 */
uint32_t RCC_GetPLLOutputClock(void)
{
    uint32_t pllsrc, pllm, plln, pllp;
    uint32_t vco_in, vco_out, pllclk;

    pllsrc = (RCC->PLLCFGR >> 22) & 0x1U;        /* 0: HSI, 1: HSE */
    pllm   = (RCC->PLLCFGR >> 0)  & 0x3FU;       /* bits 5:0 */
    plln   = (RCC->PLLCFGR >> 6)  & 0x1FFU;      /* bits 14:6 */
    pllp   = (RCC->PLLCFGR >> 16) & 0x3U;        /* bits 17:16, encoded */

    /* PLLP decoding: 00->2, 01->4, 10->6, 11->8 */
    pllp = (pllp + 1U) * 2U;

    if (pllm == 0U) return 0U; /* avoid division by zero */

    vco_in  = (pllsrc == 0U) ? (HSI_VALUE / pllm) : (HSE_VALUE / pllm);
    vco_out = vco_in * plln;
    pllclk  = vco_out / pllp;

    return pllclk;
}

/**
 * @brief  Get SYSCLK in Hz based on SWS bits (CFGR[3:2]).
 */
static uint32_t RCC_GetSystemClock(void)
{
    uint32_t sws = (RCC->CFGR >> 2) & 0x3U;  /* SWS */

    if (sws == 0U)      return HSI_VALUE;
    else if (sws == 1U) return HSE_VALUE;
    else if (sws == 2U) return RCC_GetPLLOutputClock();
    else                return HSI_VALUE;   /* fallback */
}

/**
 * @brief  Get HCLK (AHB clock) in Hz.
 */
static uint32_t RCC_GetHCLK(void)
{
    uint32_t sysclk = RCC_GetSystemClock();
    uint32_t hpre   = (RCC->CFGR >> 4) & 0xFU;  /* HPRE bits 7:4 */
    uint32_t ahb_div;

    if (hpre < 8U)
    {
        ahb_div = 1U;
    }
    else
    {
        ahb_div = AHB_Prescaler[hpre - 8U];
    }

    return sysclk / ahb_div;
}

/**
 * @brief  Get APB1 peripheral clock (PCLK1) in Hz.
 */
uint32_t RCC_GetPCLK1Value(void)
{
    uint32_t hclk  = RCC_GetHCLK();
    uint32_t ppre1 = (RCC->CFGR >> 10) & 0x7U; /* PPRE1 bits 12:10 */
    uint32_t apb_div;

    if (ppre1 < 4U)
    {
        apb_div = 1U;
    }
    else
    {
        apb_div = APB_Prescaler[ppre1 - 4U];
    }

    return hclk / apb_div;
}

/**
 * @brief  Get APB2 peripheral clock (PCLK2) in Hz.
 */
uint32_t RCC_GetPCLK2Value(void)
{
    uint32_t hclk  = RCC_GetHCLK();
    uint32_t ppre2 = (RCC->CFGR >> 13) & 0x7U; /* PPRE2 bits 15:13 */
    uint32_t apb_div;

    if (ppre2 < 4U)
    {
        apb_div = 1U;
    }
    else
    {
        apb_div = APB_Prescaler[ppre2 - 4U];
    }

    return hclk / apb_div;
}
