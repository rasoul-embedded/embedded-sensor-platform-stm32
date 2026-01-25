/* stm32f407xx_gpio_driver.h
 *
 *  Created on: Jan 21, 2026
 *      Author: rasoul
 */

#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include "stm32f407xx.h"

typedef struct
{
    uint8_t GPIO_PinNumber;
    uint8_t GPIO_PinMode;
    uint8_t GPIO_PinSpeed;
    uint8_t GPIO_PinPuPdControl;
    uint8_t GPIO_PinOpType;
    uint8_t GPIO_PinAltFunMode;
} GPIO_PinConfig_t;

typedef struct
{
    GPIO_RegDef_t   *pGPIOx;              /* GPIO port base address */
    GPIO_PinConfig_t GPIO_PinConfig;      /* pin configuration */
} GPIO_Handle_t;

/*
 * GPIO pin numbers
 */
#define GPIO_PIN_NO_0        0U
#define GPIO_PIN_NO_1        1U
#define GPIO_PIN_NO_2        2U
#define GPIO_PIN_NO_3        3U
#define GPIO_PIN_NO_4        4U
#define GPIO_PIN_NO_5        5U
#define GPIO_PIN_NO_6        6U
#define GPIO_PIN_NO_7        7U
#define GPIO_PIN_NO_8        8U
#define GPIO_PIN_NO_9        9U
#define GPIO_PIN_NO_10       10U
#define GPIO_PIN_NO_11       11U
#define GPIO_PIN_NO_12       12U
#define GPIO_PIN_NO_13       13U
#define GPIO_PIN_NO_14       14U
#define GPIO_PIN_NO_15       15U

/*
 * GPIO pin possible modes
 */
#define GPIO_MODE_IN         0U
#define GPIO_MODE_OUT        1U
#define GPIO_MODE_ALTFN      2U
#define GPIO_MODE_ANALOG     3U
#define GPIO_MODE_IT_FT      4U
#define GPIO_MODE_IT_RT      5U
#define GPIO_MODE_IT_RFT     6U

/*
 * GPIO pin possible output types
 */
#define GPIO_OP_TYPE_PP      0U
#define GPIO_OP_TYPE_OD      1U

/*
 * GPIO pin possible output speeds
 */
#define GPIO_SPEED_LOW       0U
#define GPIO_SPEED_MEDIUM    1U
#define GPIO_SPEED_FAST      2U
#define GPIO_SPEED_HIGH      3U

/*
 * GPIO pin pull up / pull down configuration macros
 */
#define GPIO_NO_PUPD         0U
#define GPIO_PIN_PU          1U
#define GPIO_PIN_PD          2U

/***********************************************************************************************
 *                              APIs supported by this driver
 **********************************************************************************************/
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

uint8_t  GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);

void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority);
void GPIO_IRQHandling(uint8_t PinNumber);

#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */
