/*
 * stm32f407xx_tim_driver.h
 *
 *  Created on: Apr 6, 2026
 *      Author: rasoul
 */

#ifndef INC_STM32F407XX_TIM_DRIVER_H_
#define INC_STM32F407XX_TIM_DRIVER_H_

#include "stm32f407xx.h"
#include <stdint.h>


typedef struct{
    uint32_t Prescaler;
    uint32_t AutoReload;
} TIM_Config_t;

typedef struct
{
    TIM_RegDef_t *pTIMx;
    TIM_Config_t TIM_Config;
} TIM_Handle_t;



void TIM_PeriClockControl(TIM_RegDef_t *pTIMx, uint8_t EnOrDi);
void TIM_Init(TIM_Handle_t *pTIMHandle);
void TIM_DeInit(TIM_RegDef_t *pTIMx);
void TIM_PeripheralControl(TIM_RegDef_t *pTIMx, uint8_t EnOrDi);
void TIM_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void TIM_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void TIM_EnableUpdateInterrupt(TIM_RegDef_t *pTIMx, uint8_t IRQNumber);




#define TIM_CR1_CEN_POS             	0
#define TIM_CR1_CEN            			(1U << TIM_CR1_CEN_POS)

#define TIM_DIER_UIE_POS		        0
#define TIM_DIER_UIE			        (1U << TIM_DIER_UIE_POS)


#define TIM_SR_UIF_POS	         	 	0
#define TIM_SR_UIF  	            	(1U << TIM_SR_UIF_POS)


#define TIM_EGR_UG_POS      	    	0
#define TIM_EGR_UG      	       		(1U << TIM_EGR_UG_POS)


#endif /* INC_STM32F407XX_TIM_DRIVER_H_ */
