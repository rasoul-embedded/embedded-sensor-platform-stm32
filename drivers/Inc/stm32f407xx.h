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



/***************************************************************************************
 * 							Peripheral Register Definition Structure
 ***************************************************************************************/



/* GPIO Register definition */

typedef struct
{
	__vo uint32_t MODER;
	__vo uint32_t OTYPER;
	__vo uint32_t OSPEEDR;
	__vo uint32_t PUPDR;
	__vo uint32_t IDR;
	__vo uint32_t ODR;
	__vo uint32_t BSRR;
	__vo uint32_t LCKR;
	__vo uint32_t AFR[2];
} GPIO_RegDef_t;


/* RCC Register definition */
typedef struct
{
	__vo uint32_t CR;
	__vo uint32_t PLLCFGR;
	__vo uint32_t CFGR;
	__vo uint32_t CIR;
	__vo uint32_t AHB1RSTR;
	__vo uint32_t AHB2RSTR;
	__vo uint32_t AHB3RSTR;
	uint32_t	RESERVED0;
	__vo uint32_t APB1RSTR;
	__vo uint32_t APB2RSTR;
	uint32_t	RESERVED1[2];
	__vo uint32_t AHB1ENR;
	__vo uint32_t AHB2ENR;
	__vo uint32_t AHB3ENR;
	uint32_t	RESERVED2;
	__vo uint32_t APB1ENR;
	__vo uint32_t APB2ENR;
	uint32_t	RESERVED3[2];
	__vo uint32_t AHB1LPENR;
	__vo uint32_t AHB2LPENR;
	__vo uint32_t AHB3LPENR;
	uint32_t	RESERVED4;
	__vo uint32_t APB1LPENR;
	__vo uint32_t APB2LPENR;
	uint32_t	RESERVED5[2];
	__vo uint32_t BDCR;
	__vo uint32_t CSR;
	uint32_t	RESERVED6[2];
	__vo uint32_t SSCGR;
	__vo uint32_t PLLI2SCFGR;
	__vo uint32_t PLLSAICFGR;
	__vo uint32_t DCKCFGR;
} RCC_RegDef_t;


/***********************************************************
 *              Peripheral definitions (typecasted)         *
 ***********************************************************/
#define GPIOA   ((GPIO_RegDef_t*)GPIOA_BASE_ADDR)
#define GPIOB   ((GPIO_RegDef_t*)GPIOB_BASE_ADDR)
#define GPIOC   ((GPIO_RegDef_t*)GPIOC_BASE_ADDR)
#define GPIOD   ((GPIO_RegDef_t*)GPIOD_BASE_ADDR)
#define GPIOE   ((GPIO_RegDef_t*)GPIOE_BASE_ADDR)
#define GPIOF   ((GPIO_RegDef_t*)GPIOF_BASE_ADDR)
#define GPIOG   ((GPIO_RegDef_t*)GPIOG_BASE_ADDR)
#define GPIOH   ((GPIO_RegDef_t*)GPIOH_BASE_ADDR)
#define GPIOI   ((GPIO_RegDef_t*)GPIOI_BASE_ADDR)

#define RCC     ((RCC_RegDef_t*)RCC_BASE_ADDR)


/***********************************************************
 *                 Clock enable macros                     *
 ***********************************************************/
/* GPIO clocks (AHB1ENR) */
#define GPIOA_PCLK_EN()     (RCC->AHB1ENR |= (1U << 0))
#define GPIOB_PCLK_EN()     (RCC->AHB1ENR |= (1U << 1))
#define GPIOC_PCLK_EN()     (RCC->AHB1ENR |= (1U << 2))
#define GPIOD_PCLK_EN()     (RCC->AHB1ENR |= (1U << 3))
#define GPIOE_PCLK_EN()     (RCC->AHB1ENR |= (1U << 4))
#define GPIOF_PCLK_EN()     (RCC->AHB1ENR |= (1U << 5))
#define GPIOG_PCLK_EN()     (RCC->AHB1ENR |= (1U << 6))
#define GPIOH_PCLK_EN()     (RCC->AHB1ENR |= (1U << 7))
#define GPIOI_PCLK_EN()     (RCC->AHB1ENR |= (1U << 8))




#endif /* INC_STM32F407XX_H_ */
