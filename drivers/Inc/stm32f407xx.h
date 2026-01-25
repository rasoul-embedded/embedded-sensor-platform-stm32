/* stm32f407xx.h
 *
 *  Created on: Jan 18, 2026
 *      Author: Rasoull
 *
 *  Purpose:
 *  Low-level device header for STM32F407xx
 *  Defines memory map, peripheral base addresses,
 *  register structures, and common macros.
 */

#ifndef INC_STM32F407XX_H_
#define INC_STM32F407XX_H_

#include <stdint.h>

#define __vo volatile

/*********************************** Processor Specific Details *******************************************/
/*
 * ARM Cortex-M4 Processor NVIC ISERx register Addresses
 */
#define NVIC_ISER0              ((__vo uint32_t*)0xE000E100)
#define NVIC_ISER1              ((__vo uint32_t*)0xE000E104)
#define NVIC_ISER2              ((__vo uint32_t*)0xE000E108)
#define NVIC_ISER3              ((__vo uint32_t*)0xE000E10C)

/*
 * ARM Cortex-M4 Processor NVIC ICERx register Addresses
 */
#define NVIC_ICER0              ((__vo uint32_t*)0xE000E180)
#define NVIC_ICER1              ((__vo uint32_t*)0xE000E184)
#define NVIC_ICER2              ((__vo uint32_t*)0xE000E188)
#define NVIC_ICER3              ((__vo uint32_t*)0xE000E18C)

/*
 * ARM Cortex-M4 Processor Priority register base address (IPR)
 * Note: keeping uint32_t* (word access).
 */
#define NVIC_IPR_BASE_ADDR      ((__vo uint32_t*)0xE000E400)

/* Compatibility alias (some code uses PR name) */
#define NVIC_PR_BASE_ADDR       NVIC_IPR_BASE_ADDR

#define NO_PR_BITS_IMPLEMENTED  4U

/****************************************
 *        Base Address of memories       *
 ****************************************/
#define FLASH_BASE_ADDR         0x08000000UL
#define SRAM1_BASE_ADDR         0x20000000UL
#define SRAM2_BASE_ADDR         0x2001C000UL
#define SRAM                    SRAM1_BASE_ADDR
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
 *                 AHB1 peripherals                        *
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

#define RCC_BASE_ADDR           (AHB1PERIPH_BASE_ADDR + 0x3800UL)

/***********************************************************
 *                 APB1 peripherals                        *
 ***********************************************************/
#define SPI2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3800UL)
#define SPI3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3C00UL)

#define USART2_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4400UL)
#define USART3_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4800UL)
#define UART4_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x4C00UL)
#define UART5_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x5000UL)

#define I2C1_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5400UL)
#define I2C2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5800UL)
#define I2C3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5C00UL)

/***********************************************************
 *                 APB2 peripherals                        *
 ***********************************************************/
#define USART1_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1000UL)
#define USART6_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1400UL)

#define SPI1_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3000UL)

#define SYSCFG_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x3800UL)
#define EXTI_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3C00UL)

/***************************************************************************************
 *                  Peripheral Register Definition Structures
 ***************************************************************************************/
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

typedef struct
{
    __vo uint32_t CR;
    __vo uint32_t PLLCFGR;
    __vo uint32_t CFGR;
    __vo uint32_t CIR;
    __vo uint32_t AHB1RSTR;
    __vo uint32_t AHB2RSTR;
    __vo uint32_t AHB3RSTR;
    uint32_t      RESERVED0;
    __vo uint32_t APB1RSTR;
    __vo uint32_t APB2RSTR;
    uint32_t      RESERVED1[2];
    __vo uint32_t AHB1ENR;
    __vo uint32_t AHB2ENR;
    __vo uint32_t AHB3ENR;
    uint32_t      RESERVED2;
    __vo uint32_t APB1ENR;
    __vo uint32_t APB2ENR;
    uint32_t      RESERVED3[2];
    __vo uint32_t AHB1LPENR;
    __vo uint32_t AHB2LPENR;
    __vo uint32_t AHB3LPENR;
    uint32_t      RESERVED4;
    __vo uint32_t APB1LPENR;
    __vo uint32_t APB2LPENR;
    uint32_t      RESERVED5[2];
    __vo uint32_t BDCR;
    __vo uint32_t CSR;
    uint32_t      RESERVED6[2];
    __vo uint32_t SSCGR;
    __vo uint32_t PLLI2SCFGR;
    __vo uint32_t PLLSAICFGR;
    __vo uint32_t DCKCFGR;
} RCC_RegDef_t;

typedef struct
{
    __vo uint32_t IMR;
    __vo uint32_t EMR;
    __vo uint32_t RTSR;
    __vo uint32_t FTSR;
    __vo uint32_t SWIER;
    __vo uint32_t PR;
} EXTI_RegDef_t;

typedef struct
{
    __vo uint32_t MEMRMP;
    __vo uint32_t PMC;
    __vo uint32_t EXTICR[4];
    __vo uint32_t CMPCR;
} SYSCFG_RegDef_t;

/***********************************************************
 *          Peripheral definitions (typecasted)             *
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
#define EXTI    ((EXTI_RegDef_t*)EXTI_BASE_ADDR)
#define SYSCFG  ((SYSCFG_RegDef_t*)SYSCFG_BASE_ADDR)

/***********************************************************
 *                 Clock enable macros                      *
 ***********************************************************/
#define GPIOA_PCLK_EN()     (RCC->AHB1ENR |= (1U << 0))
#define GPIOB_PCLK_EN()     (RCC->AHB1ENR |= (1U << 1))
#define GPIOC_PCLK_EN()     (RCC->AHB1ENR |= (1U << 2))
#define GPIOD_PCLK_EN()     (RCC->AHB1ENR |= (1U << 3))
#define GPIOE_PCLK_EN()     (RCC->AHB1ENR |= (1U << 4))
#define GPIOF_PCLK_EN()     (RCC->AHB1ENR |= (1U << 5))
#define GPIOG_PCLK_EN()     (RCC->AHB1ENR |= (1U << 6))
#define GPIOH_PCLK_EN()     (RCC->AHB1ENR |= (1U << 7))
#define GPIOI_PCLK_EN()     (RCC->AHB1ENR |= (1U << 8))

/* SYSCFG clock enable (needed for EXTI port mapping) */
#define SYSCFG_PCLK_EN()    (RCC->APB2ENR |= (1U << 14))

/***********************************************************
 *                 Register reset macros                   *
 ***********************************************************/
#define GPIOA_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 0)); (RCC->AHB1RSTR &= ~(1U << 0)); }while(0)
#define GPIOB_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 1)); (RCC->AHB1RSTR &= ~(1U << 1)); }while(0)
#define GPIOC_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 2)); (RCC->AHB1RSTR &= ~(1U << 2)); }while(0)
#define GPIOD_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 3)); (RCC->AHB1RSTR &= ~(1U << 3)); }while(0)
#define GPIOE_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 4)); (RCC->AHB1RSTR &= ~(1U << 4)); }while(0)
#define GPIOF_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 5)); (RCC->AHB1RSTR &= ~(1U << 5)); }while(0)
#define GPIOG_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 6)); (RCC->AHB1RSTR &= ~(1U << 6)); }while(0)
#define GPIOH_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 7)); (RCC->AHB1RSTR &= ~(1U << 7)); }while(0)
#define GPIOI_REG_RESET()  do{ (RCC->AHB1RSTR |=  (1U << 8)); (RCC->AHB1RSTR &= ~(1U << 8)); }while(0)

#define GPIO_BASEADDR_TO_CODE(x)            ((x==GPIOA) ? 0U :\
                                            (x==GPIOB) ? 1U :\
                                            (x==GPIOC) ? 2U :\
                                            (x==GPIOD) ? 3U :\
                                            (x==GPIOE) ? 4U :\
                                            (x==GPIOF) ? 5U :\
                                            (x==GPIOG) ? 6U :\
                                            (x==GPIOH) ? 7U :\
                                            (x==GPIOI) ? 8U : 0U)

/***********************************************************
 *                 Generic macros                          *
 ***********************************************************/
#define ENABLE          1U
#define DISABLE         0U
#define SET             ENABLE
#define RESET           DISABLE

#define GPIO_PIN_SET    SET
#define GPIO_PIN_RESET  RESET

/***********************************************************
 *                 IRQ numbers (NVIC)                      *
 ***********************************************************/
#define IRQ_NO_EXTI0            6U
#define IRQ_NO_EXTI1            7U
#define IRQ_NO_EXTI2            8U
#define IRQ_NO_EXTI3            9U
#define IRQ_NO_EXTI4            10U
#define IRQ_NO_EXTI9_5          23U
#define IRQ_NO_EXTI15_10        40U

#endif /* INC_STM32F407XX_H_ */
