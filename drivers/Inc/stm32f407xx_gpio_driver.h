/*
 * stm32f407xx_gpio_driver.h
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
}GPIO_PinConfig_t;




/*
 *	Handle structure for GPIO pin
 */


typedef struct
{
	GPIO_RegDef_t *pGpiox;				//pointer to hold the base address of the GPIO peripheral
	GPIO_PinConfig_t GPIO_PinConfig;	//Holds GPIO pin configuration settings
}GPIO_Handle_t;



/***********************************************************************************************
 * 								APIs supported by this driver
 *
 **********************************************************************************************/


/**
 * @brief  Initializes a GPIO pin according to the configuration in the GPIO handle.
 *
 * This function configures:
 * - Mode (input/output/alternate/analog/interrupt)
 * - Output type (push-pull/open-drain) if output/alt
 * - Speed if output/alt
 * - Pull-up / pull-down configuration
 * - Alternate function selection if mode is alternate
 *
 * @param  pGPIOHandle Pointer to GPIO handle structure containing:
 *         - pGPIOx: GPIO port base address
 *         - GPIO_PinConfig: pin configuration parameters
 *
 * @return None
 *
 * @note   Before calling this function, ensure the GPIO peripheral clock is enabled.
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);

/**
 * @brief  De-initializes the given GPIO port registers (resets the port).
 *
 * This function resets the complete GPIO port peripheral registers to default
 * reset values using the RCC reset mechanism.
 *
 * @param  pGPIOx Base address of the GPIO port.
 *
 * @return None
 *
 * @note   This affects the whole port (all pins), not a single pin.
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

/**
 * @brief  Reads the logic level present on a specific GPIO input pin.
 *
 * Reads the IDR (Input Data Register) bit corresponding to PinNumber.
 *
 * @param  pGPIOx Base address of the GPIO port.
 * @param  PinNumber GPIO pin number (0 to 15).
 *
 * @return uint8_t Pin state:
 *         - 0: logic low
 *         - 1: logic high
 *
 * @note   Pin should be configured as input or interrupt mode.
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

/**
 * @brief  Reads the 16-bit value of the input port (IDR register).
 *
 * @param  pGPIOx Base address of the GPIO port.
 *
 * @return uint16_t 16-bit port value representing IDR[15:0].
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);

/**
 * @brief  Writes a logic level to a specific GPIO output pin.
 *
 * Typically modifies the ODR bit corresponding to the PinNumber.
 * (Some implementations may prefer BSRR for atomic set/reset.)
 *
 * @param  pGPIOx Base address of the GPIO port.
 * @param  PinNumber GPIO pin number (0 to 15).
 * @param  value Value to write:
 *         - 0: set pin low
 *         - 1: set pin high
 *
 * @return None
 *
 * @note   Pin should be configured as output mode (or alternate mode if allowed by design).
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t value);

/**
 * @brief  Writes a 16-bit value to the GPIO output port (ODR register).
 *
 * @param  pGPIOx Base address of the GPIO port.
 * @param  value 16-bit value to be written to ODR[15:0].
 *
 * @return None
 *
 * @note   This updates all 16 pins at once. Use carefully if some pins are used by other logic.
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t value);

/**
 * @brief  Toggles the current state of a specific GPIO output pin.
 *
 * Typically XORs the corresponding ODR bit.
 *
 * @param  pGPIOx Base address of the GPIO port.
 * @param  PinNumber GPIO pin number (0 to 15).
 *
 * @return None
 *
 * @note   Pin should be configured as output mode.
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

/**
 * @brief  Configures the NVIC for a given IRQ number (enable/disable and priority).
 *
 * This function typically:
 * - Enables/disables the IRQ line in NVIC ISER/ICER
 * - Programs the priority in NVIC priority registers
 *
 * @param  IRQNumber IRQ number to configure (MCU-specific value).
 * @param  IRQPriority Priority level to set (0 = highest priority in many schemes).
 * @param  EnOrDi Enable or disable the interrupt:
 *         - 1: enable
 *         - 0: disable
 *
 * @return None
 *
 * @note   This configures NVIC only. You still must configure EXTI line + GPIO mode for interrupts.
 */
void GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t EnOrDi);

/**
 * @brief  Handles the GPIO interrupt for a given pin number.
 *
 * This function typically clears the EXTI pending bit for the given pin,
 * and may invoke a user callback (if implemented).
 *
 * @param  PinNumber GPIO pin number (0 to 15) whose EXTI line triggered.
 *
 * @return None
 *
 * @note   Ensure this function is called from the corresponding IRQHandler
 *         (e.g., EXTI0_IRQHandler calls GPIO_IRQHandling(0)).
 */
void GPIO_IRQHandling(uint8_t PinNumber);



#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */
