/*
 * stm32f407xx_rcc_driver.h
 *
 *  Created on: Feb 3, 2026
 *      Author: rasoul
 */

#ifndef INC_STM32F407XX_RCC_DRIVER_H_
#define INC_STM32F407XX_RCC_DRIVER_H_

#include "stm32f407xx.h"


/************************************************************************************
 * RCC APIs
 ************************************************************************************/


// Get the PLL output clock frequency (PLLCLK).
uint32_t RCC_GetPLLOutputClock(void);


//Get APB1 peripheral clock frequency (PCLK1).
uint32_t RCC_GetPCLK1Value(void);


//Get APB2 peripheral clock frequency (PCLK2).
uint32_t RCC_GetPCLK2Value(void);


#endif /* INC_STM32F407XX_RCC_DRIVER_H_ */
