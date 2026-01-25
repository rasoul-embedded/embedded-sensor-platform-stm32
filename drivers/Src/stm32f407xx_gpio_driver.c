/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: Jan 21, 2026
 *      Author: Rasoul
 */

#include "stm32f407xx_gpio_driver.h"

void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{
    /*********************************************************
     * 1) ENABLE CLOCK FOR THE SELECTED GPIO PORT
     *********************************************************/
    if (pGPIOHandle->pGPIOx == GPIOA)      { GPIOA_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOB) { GPIOB_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOC) { GPIOC_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOD) { GPIOD_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOE) { GPIOE_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOF) { GPIOF_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOG) { GPIOG_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOH) { GPIOH_PCLK_EN(); }
    else if (pGPIOHandle->pGPIOx == GPIOI) { GPIOI_PCLK_EN(); }

    uint32_t temp = 0U;

    /*********************************************************
     * 2) CONFIGURE PIN MODE
     *********************************************************/
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
    {
        temp = ((uint32_t)pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <<
               (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

        pGPIOHandle->pGPIOx->MODER &= ~(0x3U <<
               (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

        pGPIOHandle->pGPIOx->MODER |= temp;
        temp = 0U;
    }
    else
    {
        /*********************************************************
         * INTERRUPT MODE CONFIGURATION
         *********************************************************/

        /* Force pin to input mode (00) for EXTI usage */
        pGPIOHandle->pGPIOx->MODER &= ~(0x3U <<
               (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

        /* 2.1 Configure trigger selection */
        if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
        {
            EXTI->FTSR |=  (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->RTSR &= ~(1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }
        else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
        {
            EXTI->FTSR &= ~(1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->RTSR |=  (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }
        else /* GPIO_MODE_IT_RFT */
        {
            EXTI->FTSR |=  (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->RTSR |=  (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }

        /* 2.2 Configure GPIO port selection in SYSCFG EXTICR */
        uint32_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4U;
        uint32_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4U;

        uint32_t portcode = (uint32_t)GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);

        SYSCFG_PCLK_EN();

        SYSCFG->EXTICR[temp1] &= ~(0xFU << (4U * temp2));
        SYSCFG->EXTICR[temp1] |=  (portcode << (4U * temp2));

        /* 2.3 Unmask EXTI line */
        EXTI->IMR |= (1U << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
    }

    /*********************************************************
     * 3) CONFIGURE OUTPUT SPEED (OSPEEDR)
     *********************************************************/
    temp = ((uint32_t)pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed <<
           (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3U <<
           (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->OSPEEDR |= temp;
    temp = 0U;

    /*********************************************************
     * 4) CONFIGURE PULL-UP / PULL-DOWN (PUPDR)
     *********************************************************/
    temp = ((uint32_t)pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl <<
           (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3U <<
           (2U * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->PUPDR |= temp;
    temp = 0U;

    /*********************************************************
     * 5) CONFIGURE OUTPUT TYPE (OTYPER)
     * Only meaningful for output/alternate, but harmless otherwise.
     *********************************************************/
    temp = ((uint32_t)pGPIOHandle->GPIO_PinConfig.GPIO_PinOpType <<
           (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->OTYPER &= ~(0x1U <<
           pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

    pGPIOHandle->pGPIOx->OTYPER |= temp;
    temp = 0U;

    /*********************************************************
     * 6) CONFIGURE ALTERNATE FUNCTION (AFR)
     *********************************************************/
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
    {
        uint32_t reg_idx   = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8U;
        uint32_t bit_pos   = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8U;

        pGPIOHandle->pGPIOx->AFR[reg_idx] &= ~(0xFU << (4U * bit_pos));
        pGPIOHandle->pGPIOx->AFR[reg_idx] |=
            ((uint32_t)pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4U * bit_pos));
    }
}

void GPIO_DeInit(GPIO_RegDef_t *pGPIOx)
{
    if (pGPIOx == GPIOA)      { GPIOA_REG_RESET(); }
    else if (pGPIOx == GPIOB) { GPIOB_REG_RESET(); }
    else if (pGPIOx == GPIOC) { GPIOC_REG_RESET(); }
    else if (pGPIOx == GPIOD) { GPIOD_REG_RESET(); }
    else if (pGPIOx == GPIOE) { GPIOE_REG_RESET(); }
    else if (pGPIOx == GPIOF) { GPIOF_REG_RESET(); }
    else if (pGPIOx == GPIOG) { GPIOG_REG_RESET(); }
    else if (pGPIOx == GPIOH) { GPIOH_REG_RESET(); }
    else if (pGPIOx == GPIOI) { GPIOI_REG_RESET(); }
}

uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
    return (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x1U);
}

uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx)
{
    return (uint16_t)(pGPIOx->IDR & 0xFFFFU);
}

void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t value)
{
    /* Use BSRR for atomic set/reset */
    if (value == GPIO_PIN_SET)
    {
        pGPIOx->BSRR = (1U << PinNumber);
    }
    else
    {
        pGPIOx->BSRR = (1U << (PinNumber + 16U));
    }
}

void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t value)
{
    pGPIOx->ODR = (uint32_t)value;
}

void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
    pGPIOx->ODR ^= (1U << PinNumber);
}

void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        if (IRQNumber <= 31U)
        {
            *NVIC_ISER0 = (1U << IRQNumber);
        }
        else if (IRQNumber < 64U)
        {
            *NVIC_ISER1 = (1U << (IRQNumber % 32U));
        }
        else if (IRQNumber < 96U)
        {
            *NVIC_ISER2 = (1U << (IRQNumber % 32U));
        }
        else
        {
            *NVIC_ISER3 = (1U << (IRQNumber % 32U));
        }
    }
    else
    {
        if (IRQNumber <= 31U)
        {
            *NVIC_ICER0 = (1U << IRQNumber);
        }
        else if (IRQNumber < 64U)
        {
            *NVIC_ICER1 = (1U << (IRQNumber % 32U));
        }
        else if (IRQNumber < 96U)
        {
            *NVIC_ICER2 = (1U << (IRQNumber % 32U));
        }
        else
        {
            *NVIC_ICER3 = (1U << (IRQNumber % 32U));
        }
    }
}

void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority)
{
    uint8_t iprx         = IRQNumber / 4U;
    uint8_t iprx_section = IRQNumber % 4U;

    uint8_t shift_amount = (8U * iprx_section) + (8U - NO_PR_BITS_IMPLEMENTED);

    __vo uint32_t *ipr = (NVIC_IPR_BASE_ADDR + iprx);

    /* Clear existing priority bits (implemented bits only) */
    *ipr &= ~(((uint32_t)0xFU) << shift_amount);

    /* Set new priority (mask to implemented bits) */
    *ipr |= (((uint32_t)(IRQPriority & 0xFU)) << shift_amount);
}

void GPIO_IRQHandling(uint8_t PinNumber)
{
    if (EXTI->PR & (1U << PinNumber))
    {
        /* Clear pending bit by writing 1 */
        EXTI->PR |= (1U << PinNumber);
    }
}
