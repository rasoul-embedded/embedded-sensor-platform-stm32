/*
 * stm32f407xx.h
 *
 *  Created on: Jan 18, 2026
 *      Author: Rasoull
 */

#ifndef INC_STM32F407XX_H_
#define INC_STM32F407XX_H_

#include <stdint.h>

/* Common helper macros */
#define __vo volatile

/****************************************
 *        Base Address of memories      *
 ****************************************/

/* Flash memory address */
#define FLASH_BASE_ADDR         0x08000000UL

/* SRAM memories address */
#define SRAM1_BASE_ADDR         0x20000000UL
#define SRAM2_BASE_ADDR         0x2001C000UL
#define SRAM                    SRAM1_BASE_ADDR

/* System memory (boot ROM) */
#define ROM_BASE_ADDR           0x1FFF0000UL


/***********************************************************
 *      AHBx and APBx Bus Peripheral base addresses         *
 ***********************************************************/
#define PERIPH_BASE_ADDR        0x40000000UL
#define APB1PERIPH_BASE_ADDR    PERIPH_BASE_ADDR
#define APB2PERIPH_BASE_ADDR    0x40010000UL
#define AHB1PERIPH_BASE_ADDR    0x40020000UL
#define AHB2PERIPH_BASE_ADDR    0x50000000UL

/***********************************************************
 *                 AHB1 peripherals                         *
 ***********************************************************/
#define GPIOA_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0000UL)
#define GPIOB_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0400UL)
#define GPIOC_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0800UL)
#define GPIOD_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0C00UL)
#define GPIOE_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1000UL)
#define GPIOF_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1400UL)
#define GPIOG_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1800UL)
#define GPIOH_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1C00UL)
#define GPIOI_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x2000UL)


#define RCC_BASE_ADDR           (AHB1PERIPH_BASE_ADDR + 0x3800UL)  /* 0x40023800 */


/***********************************************************
 *                 APB1 peripherals                         *
 ***********************************************************/
#define SPI2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3800UL)  /* 0x40003800 */
#define SPI3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3C00UL)  /* 0x40003C00 */

#define USART2_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4400UL)  /* 0x40004400 */
#define USART3_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4800UL)  /* 0x40004800 */
#define UART4_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x4C00UL)  /* 0x40004C00 */
#define UART5_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x5000UL)  /* 0x40005000 */

#define I2C1_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5400UL)  /* 0x40005400 */
#define I2C2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5800UL)  /* 0x40005800 */
#define I2C3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5C00UL)  /* 0x40005C00 */


/***********************************************************
 *                 APB2 peripherals                         *
 ***********************************************************/
#define USART1_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1000UL)  /* 0x40011000 */
#define USART6_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1400UL)  /* 0x40011400 */

#define SPI1_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3000UL)  /* 0x40013000 */

#define SYSCFG_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x3800UL)  /* 0x40013800 */
#define EXTI_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3C00UL)  /* 0x40013C00 */


#endif /* INC_STM32F407XX_H_ */
