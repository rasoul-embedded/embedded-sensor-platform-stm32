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
    uint16_t Prescaler;
    uint16_t AutoReload;
    uint8_t	UpdateInterruptEnable;
} TIM_Basic_Config_t;

typedef struct
{
	TIM_Basic_RegDef_t *pTIMx;
	TIM_Basic_Config_t TIM_Config;
} TIM_Basic_Handle_t;

typedef struct{
    uint16_t Prescaler;
    uint32_t AutoReload;
    uint8_t	UpdateInterruptEnable;
} TIM_GeneralPurpose_Config_t;



typedef struct
{
	TIM_GeneralPurpose_RegDef_t *pTIMx;
	TIM_GeneralPurpose_Config_t TIM_Config;
} TIM_GeneralPurpose_Handle_t;


void TIM_Basic_PeriClockControl(TIM_Basic_RegDef_t *pTIMx, uint8_t EnOrDi);
void TIM_Basic_Init(TIM_Basic_Handle_t *pTIMHandle);
void TIM_Basic_DeInit(TIM_Basic_RegDef_t *pTIMx);
void TIM_Basic_PeripheralControl(TIM_Basic_RegDef_t *pTIMx, uint8_t EnOrDi);
void TIM_Basic_EnableUpdateInterrupt(TIM_Basic_RegDef_t *pTIMx, uint8_t EnOrDi);



void TIM_GeneralPurpose_PeriClockControl(TIM_GeneralPurpose_RegDef_t *pTIMx, uint8_t EnOrDi);
void TIM_GeneralPurpose_Init(TIM_GeneralPurpose_Handle_t *pTIMHandle);
void TIM_GeneralPurpose_DeInit(TIM_GeneralPurpose_RegDef_t *pTIMx);
void TIM_GeneralPurpose_PeripheralControl(TIM_GeneralPurpose_RegDef_t *pTIMx, uint8_t EnOrDi);
void TIM_GeneralPurpose_EnableUpdateInterrupt(TIM_GeneralPurpose_RegDef_t *pTIMx, uint8_t EnOrDi);


void TIM_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void TIM_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);


void TIM_Basic_IRQHandling(TIM_Basic_Handle_t *pTIMHandle);
void TIM_Basic_ApplicationEventCallback(TIM_Basic_Handle_t *TIMHandle);

void TIM_GeneralPurpose_IRQHandling(TIM_GeneralPurpose_Handle_t *pTIMHandle);
void TIM_GeneralPurpose_ApplicationEventCallback(TIM_GeneralPurpose_Handle_t *TIMHandle);


#define TIM_CR1_CEN_POS             	0
#define TIM_CR1_CEN            			(1U << TIM_CR1_CEN_POS)

#define TIM_DIER_UIE_POS		        0
#define TIM_DIER_UIE			        (1U << TIM_DIER_UIE_POS)


#define TIM_SR_UIF_POS	         	 	0
#define TIM_SR_UIF  	            	(1U << TIM_SR_UIF_POS)


#define TIM_EGR_UG_POS      	    	0
#define TIM_EGR_UG      	       		(1U << TIM_EGR_UG_POS)


#endif /* INC_STM32F407XX_TIM_DRIVER_H_ */
